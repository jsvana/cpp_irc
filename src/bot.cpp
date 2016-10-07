#include "bot.h"

#include <iostream>

std::tuple<std::string, std::vector<std::string>> Command::parse_command(const Message& message) {
  auto last_arg = message.params[1];
  auto i = last_arg.find(' ');
  std::string cmd;
  if (i == std::string::npos) {
    cmd = last_arg.substr(1);
    i = last_arg.length();
  } else {
    cmd = last_arg.substr(1, i - 1);
  }

  i++;

  std::vector<std::string> args;
  while (i < last_arg.length()) {
    auto end = last_arg.find(' ', i);
    if (end == std::string::npos) {
      args.push_back(last_arg.substr(i));
      break;
    }
    args.push_back(last_arg.substr(i, end - i));
    i = end + 1;
  }

  return std::make_tuple(cmd, args);
}

void Bot::add_callbacks() {
  // ISUPPORT (doubling as connected)
  client_.add_callback("005", [this](const Message&) {
    while (!this->starting_channels.empty()) {
      this->join(this->starting_channels.front());
      this->starting_channels.pop();
    }
  });

  // Nick in use
  client_.add_callback("433", [this](const Message&) {
    if (this->nick_choices.empty()) {
      throw std::out_of_range("Out of nick choices");
    }
    this->nick_choices.pop();
    this->nick(this->nick_choices.front());
  });

  client_.add_callback("PRIVMSG", [this](const Message& message) {
    if (message.params.size() < 2) {
      std::cerr << "[BADC] " << message.line << std::endl;
      return;
    }

    if (message.params[1][0] != this->command_prefix_) {
      return;
    }

    std::string origin;
    if (message.params[0] == this->nick()) {
      origin = *message.user;
    } else {
      origin = message.params[0];
    }

    Command command(message, origin);

    const auto& callbacks = this->callbacks_for_command(command.command);
    if (!callbacks) {
      std::cout << "NO CALLBACKS FOR " << command.command << std::endl;
      return;
    }

    for (const auto& callback : *callbacks) {
      callback(this, message, command);
    }
  });
}

void Bot::add_command_callback(const std::string& command, const CommandCallback callback) {
  callbacks_[command].push_back(callback);
}

boost::optional<const std::vector<CommandCallback>&> Bot::callbacks_for_command(const std::string& command) {
  const auto iter = callbacks_.find(command);
  if (iter == callbacks_.end()) {
    return boost::none;
  }

  return iter->second;
}
