#include "client.h"

#include <algorithm>
#include <iostream>

void dispatcher(Client &client) {
  client.write("USER jsvana 0.0.0.0 0.0.0.0 :jsvana test");
  client.nick(client.nick_choices.front());

  while (true) {
    auto message = client.read();

    const auto &callbacks = client.callbacks_for_command(message.command());
    for (const auto &callback : callbacks) {
      callback(client, message);
    }
  }
}

Client::Client(const std::string &host, const std::string &port) {
  sock_ = std::make_unique<IrcSocket>(host, port);
}

void Client::add_callback(const std::string &command, const std::function<void(Client &, const Message &)> callback) {
  auto iter = callbacks_.find(command);
  if (iter == callbacks_.end()) {
    callbacks_.emplace(std::make_pair(command, std::vector<std::function<void(Client &, const Message &)>>()));
  }
  callbacks_[command].push_back(callback);
}

const std::vector<std::function<void(Client &, const Message &)>> &Client::callbacks_for_command(const std::string &command) {
  auto iter = callbacks_.find(command);
  if (iter == callbacks_.end()) {
    return EMPTY_CALLBACKS;
  }

  return iter->second;
}

void Client::setup_callbacks() {
  // ISUPPORT (doubling as connected)
  add_callback("005", [](Client &client, const Message &message) {
    // Parse valid user prefixes for the server
    for (const auto &param : message.params()) {
      if (param.find("PREFIX=") != 0) {
        continue;
      }
      std::size_t i = param.find('(') + 1;
      std::size_t j = param.find(')') + 1;

      while (param[i] != ')') {
        UserMode mode;
        switch (param[i]) {
        case 'q':
          mode = MODE_OWNER;
          break;
        case 'a':
          mode = MODE_ADMIN;
          break;
        case 'o':
          mode = MODE_OP;
          break;
        case 'h':
          mode = MODE_HALFOP;
          break;
        case 'v':
          mode = MODE_VOICE;
          break;
        default:
          mode = MODE_UNKNOWN;
          break;
        }
        client.add_prefix_mapping(param[j], mode);
        i += 1;
        j += 1;
      }
    }

    client.join("#encoded-test");
  });

  // Nick in use
  add_callback("433", [](Client &client, const Message &) {
    if (client.nick_choices.empty()) {
      throw "Out of nick choices";
    }
    client.nick_choices.pop();
    client.nick(client.nick_choices.front());
  });

  add_callback("PING", [](Client &client, const Message &message) {
    client.write("PONG " + message.params()[0]);
  });

  add_callback("JOIN", [](Client &client, const Message &message) {
    if (message.prefix().entity() == client.nick()) {
      client.add_channel(message.params()[0]);
      std::cout << "Active channels:";
      for (const auto &p : client.channels()) {
        std::cout << " " << p.first;
      }
      std::cout << std::endl;
    }

    client.channel_message("#encoded-test", "test message woo");
  });

  // Topic
  add_callback("332", [](Client &client, const Message &message) {
    auto channel = client.get_channel(message.params()[1]);
    channel->set_topic(message.params()[2]);
  });

  // Names
  add_callback("353", [](Client &client, const Message &message) {
    auto channel = client.get_channel(message.params()[2]);
    auto nicks_str = message.params()[3];

    std::size_t i = 0;
    std::size_t end;
    while (i < nicks_str.length()) {
      end = nicks_str.find(' ', i);
      auto nick = nicks_str.substr(i, end - i);
      i = end + 1;

      bool added = false;
      for (const auto &p : client.prefixes()) {
        if (nick[0] == p.first) {
          channel->add_user(nick.substr(1), p.second);
          added = true;
        }
      }

      if (!added) {
        channel->add_user(nick, MODE_UNKNOWN);
      }
    }

    std::cout << "CHANNEL " << channel->name() << ":" << std::endl;
    std::cout << "  TOPIC: " << channel->topic() << std::endl;
    std::cout << "  NICKS:";
    for (const auto &user : channel->users()) {
      std::cout << " " << user.first;
    }
    std::cout << std::endl;
  });
}

void Client::run() {
  setup_callbacks();

  std::thread dispatcher_thread(dispatcher, std::ref(*this));

  sock_->run();

  dispatcher_thread.join();
}

Channel *Client::get_channel(const std::string &channel) {
  auto iter = channels_.find(channel);
  if (iter != channels_.end()) {
    return &iter->second;
  }

  channels_.emplace(std::make_pair(channel, Channel(channel)));
  return &channels_.find(channel)->second;
}

void Client::join(const std::string &channel) {
  write("JOIN " + channel);
}

void Client::nick(const std::string &nick) {
  write("NICK " + nick);
}

bool Client::channel_message(const std::string &channel, const std::string &message) {
  auto iter = channels_.find(channel);
  if (iter == channels_.end()) {
    std::cerr << "Unknown channel \"" << channel << "\"" << std::endl;
    return false;
  }

  write("PRIVMSG " + channel + " :" + message);
  return true;
}

Message Client::read() {
  // Read in batches because not all lines will end in \r\n
  std::size_t end;
  std::string line;
  do {
    end = unfinished_line_.find("\r\n");
    if (end == std::string::npos) {
      unfinished_line_ += sock_->read_queue().pop();
    }
  } while (end == std::string::npos);

  line = unfinished_line_.substr(0, end);
  unfinished_line_ = unfinished_line_.substr(end + 2);

  std::cout << "RECV " << line << std::endl;

  return Message(line);
}
