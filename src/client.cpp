#include "client.h"

#include <algorithm>
#include <stdexcept>
#include <iostream>

void dispatcher(Client &client) {
  client.write("USER jsvana 0.0.0.0 0.0.0.0 :jsvana test");
  client.nick(client.nick_choices.front());

  while (true) {
    const auto message = client.read();

    const auto &callbacks = client.callbacks_for_command(message.command);
    if (!callbacks) {
      continue;
    }

    for (const auto &callback : *callbacks) {
      callback(message);
    }
  }
}

void Client::add_callback(const std::string &command, const MessageCallback callback) {
  callbacks_[command].push_back(callback);
}

boost::optional<const std::vector<MessageCallback> &> Client::callbacks_for_command(const std::string &command) {
  const auto iter = callbacks_.find(command);
  if (iter == callbacks_.end()) {
    return boost::none;
  }

  return iter->second;
}

void Client::setup_callbacks() {
  // ISUPPORT (doubling as connected)
  add_callback("005", [this](const Message &message) {
    // Parse valid user prefixes for the server
    for (const auto &param : message.params) {
      if (param.find("PREFIX=") != 0) {
        continue;
      }
      std::size_t i = param.find('(') + 1;
      std::size_t j = param.find(')') + 1;

      while (param[i] != ')') {
        this->add_prefix_mapping(param[j], static_cast<UserMode>(param[i]));
        i += 1;
        j += 1;
      }
    }

    this->join("#encoded-test");
  });

  // Nick in use
  add_callback("433", [this](const Message &) {
    if (this->nick_choices.empty()) {
      throw std::out_of_range("Out of nick choices");
    }
    this->nick_choices.pop();
    this->nick(this->nick_choices.front());
  });

  add_callback("PING", [this](const Message &message) {
    this->write("PONG " + message.params[0]);
  });

  add_callback("JOIN", [this](const Message &message) {
    if (message.entity == this->nick()) {
      this->add_channel(message.params[0]);
      std::cout << "Active channels:";
      for (const auto &p : this->channels()) {
        std::cout << " " << p.first;
      }
      std::cout << std::endl;
    }

    this->channel_message("#encoded-test", "test message woo");
  });

  // Topic
  add_callback("332", [this](const Message &message) {
    auto channel = this->get_channel(message.params[1]);
    channel->set_topic(message.params[2]);
  });

  // Names
  add_callback("353", [this](const Message &message) {
    auto channel = this->get_channel(message.params[2]);
    auto nicks_str = message.params[3];

    std::size_t i = 0;
    std::size_t end;
    while (i < nicks_str.length()) {
      end = nicks_str.find(' ', i);
      auto nick = nicks_str.substr(i, end - i);
      i = end + 1;

      bool added = false;
      for (const auto &p : this->prefixes()) {
        if (nick[0] == p.first) {
          channel->add_user(nick.substr(1), p.second);
          added = true;
        }
      }

      if (!added) {
        channel->add_user(nick);
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

  sock_.run();

  dispatcher_thread.join();
}

Channel *Client::get_channel(const std::string &channel) {
  auto iter = channels_.find(channel);
  if (iter != channels_.end()) {
    return &iter->second;
  }

  channels_.emplace(channel, channel);
  return &channels_.find(channel)->second;
}

void Client::join(const std::string &channel) {
  // Make sure we don't double join
  if (channels_.find(channel) != channels_.end()) {
    return;
  }
  for (const auto &pending : pending_joins_) {
    if (pending == channel) {
      return;
    }
  }

  write("JOIN " + channel);
  pending_joins_.push_back(channel);
}

void Client::nick(const std::string &nick) {
  write("NICK " + nick);
}

bool Client::channel_message(const std::string &channel, const std::string &message) {
  if (channels_.find(channel) == channels_.end()) {
    std::cerr << "Unknown channel \"" << channel << "\"" << std::endl;
    return false;
  }

  write("PRIVMSG " + channel + " :" + message);
  return true;
}

void Client::add_channel(const std::string &channel) {
  channels_.emplace(std::make_pair(channel, Channel(channel)));
  pending_joins_.remove(channel);
}

Message Client::read() {
  // Read in batches because not all lines will end in \r\n
  std::size_t end;
  std::string line;
  while (end = unfinished_line_.find("\r\n"), end == std::string::npos) {
    unfinished_line_ += sock_.read_queue().pop();
  }

  line = unfinished_line_.substr(0, end);
  unfinished_line_ = unfinished_line_.substr(end + 2);

  std::cout << "RECV " << line << std::endl;

  return {line};
}
