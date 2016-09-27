#include "client.h"

#include <algorithm>
#include <iostream>

void dispatcher(Client &client) {
  client.write_lines({
    "USER jsvana 0.0.0.0 0.0.0.0 :jsvana test",
    "NICK " + client.nick_choices.front(),
  });

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
  add_callback("005", [](Client &client, const Message &) {
    client.write("JOIN #encoded-test");
  });

  // Nick in use
  add_callback("433", [](Client &client, const Message &) {
    if (client.nick_choices.empty()) {
      throw "Out of nick choices";
    }
    client.nick_choices.pop();
    client.write("NICK " + client.nick_choices.front());
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
    std::set<std::string> nicks;

    std::size_t i = 0;
    std::size_t end;
    while (i < nicks_str.length()) {
      end = nicks_str.find(' ', i);
      nicks.insert(nicks_str.substr(i, end - i));
      i = end + 1;
    }
    channel->add_nicks(nicks);

    std::cout << "CHANNEL " << channel->name() << ":" << std::endl;
    std::cout << "  TOPIC: " << channel->topic() << std::endl;
    std::cout << "  NICKS:";
    for (const auto &nick : channel->nicks()) {
      std::cout << " " << nick;
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
