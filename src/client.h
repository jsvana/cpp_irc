#pragma once

#include "channel.h"
#include "irc_socket.h"
#include "message.h"

#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <vector>

class Client {
 private:
  std::unique_ptr<IrcSocket> sock_;
  std::map<std::string, std::vector<std::function<void(Client &, const Message &)>>> callbacks_;

  const std::vector<std::function<void(Client &, const Message &)>> EMPTY_CALLBACKS;

  std::map<std::string, Channel> channels_;
  std::map<char, UserMode> prefixes_;

  void setup_callbacks();

  std::string unfinished_line_;

 public:
  std::queue<std::string> nick_choices;

  Client(const std::string &host, const std::string &port);

  IrcSocket &socket() { return *sock_; }

  void add_prefix_mapping(char prefix, UserMode mode) { prefixes_[prefix] = mode; }

  const std::map<char, UserMode> &prefixes() const { return prefixes_; }

  void set_nick_choices(const std::vector<std::string> &choices) {
    for (const auto &choice : choices) {
      nick_choices.push(choice);
    }
  }

  const std::string &nick() const { return nick_choices.front(); }

  void add_callback(const std::string &command, const std::function<void(Client &, const Message &)> callback);

  bool connect() { return sock_->connect(); }

  void run();

  void write(const std::string &message) { sock_->write(message); }

  void write_lines(const std::vector<std::string> &lines) {
    sock_->write_lines(lines);
  }

  Channel *get_channel(const std::string &channel);

  void add_channel(const std::string &channel) { channels_.emplace(std::make_pair(channel, Channel(channel))); }

  const std::vector<std::function<void(Client &, const Message &)>> &callbacks_for_command(const std::string &command);

  const std::map<std::string, Channel> &channels() { return channels_; }

  Message read();
};
