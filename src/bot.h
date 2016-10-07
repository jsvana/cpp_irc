#pragma once

#include "client.h"

#include <functional>
#include <queue>
#include <string>
#include <tuple>
#include <vector>

class Command {
 private:
  const std::tuple<std::string, std::vector<std::string>> command_parts_;
  std::tuple<std::string, std::vector<std::string>> parse_command(const Message& message);

 public:
  const std::string origin;
  const std::string command;
  const std::vector<std::string> args;

  Command(const Message& message, const std::string& orig)
    : command_parts_(parse_command(message)),
      origin(orig),
      command(std::get<0>(command_parts_)),
      args(std::get<1>(command_parts_)) {}
};

class Bot;

typedef std::function<void(Bot*, const Message&, const Command&)> CommandCallback;

class Bot {
 private:
  Client client_;

  char command_prefix_;

  std::unordered_map<std::string, std::vector<CommandCallback>> callbacks_;

 public:
  std::queue<std::string> nick_choices;
  std::queue<std::string> starting_channels;

  Bot(const std::string& host, const std::string& port, char prefix) : client_(host, port), command_prefix_(prefix) {}

  void set_nick_choices(const std::vector<std::string>& choices) {
    for (const auto& choice : choices) {
      nick_choices.push(choice);
    }
  }

  void add_starting_channels(const std::vector<std::string>& channels) {
    for (const auto& channel : channels) {
      starting_channels.push(channel);
    }
  }

  void add_callbacks();

  void add_callback(const std::string& command, const MessageCallback callback) {
    client_.add_callback(command, callback);
  }

  void add_command_callback(const std::string &command, const CommandCallback callback);
  boost::optional<const std::vector<CommandCallback>&> callbacks_for_command(const std::string& command);

  const std::string& nick() const { return nick_choices.front(); }

  bool channel_message(const std::string& channel, const std::string& message) {
    return client_.channel_message(channel, message);
  }

  void join(const std::string& channel) {
    client_.join(channel);
  }

  void nick(const std::string& nick) {
    client_.nick(nick);
  }

  bool connect() { return client_.connect(); }

  void run() {
    add_callbacks();

    client_.write("USER jsvana 0.0.0.0 0.0.0.0 :jsvana test");
    client_.nick(nick_choices.front());

    client_.run();
  }
};
