#pragma once

#include "channel.h"
#include "irc_socket.h"
#include "message.h"

#include <boost/optional.hpp>

#include <functional>
#include <list>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

typedef std::function<void(const Message &)> MessageCallback;

class Client {
 private:
  IrcSocket sock_;
  std::unordered_map<std::string, std::vector<MessageCallback>> callbacks_;

  std::list<std::string> pending_joins_;

  std::unordered_map<std::string, Channel> channels_;
  std::unordered_map<char, UserMode> prefixes_;

  void setup_callbacks();

  std::string unfinished_line_;

 public:
  std::queue<std::string> nick_choices;

  Client(const std::string &host, const std::string &port) : sock_(host, port) {}

  IrcSocket &socket() { return sock_; }

  void add_prefix_mapping(char prefix, UserMode mode) { prefixes_[prefix] = mode; }

  const std::unordered_map<char, UserMode> &prefixes() const { return prefixes_; }

  void set_nick_choices(const std::vector<std::string> &choices) {
    for (const auto &choice : choices) {
      nick_choices.push(choice);
    }
  }

  const std::string &nick() const { return nick_choices.front(); }

  void add_callback(const std::string &command, const MessageCallback callback);

  bool connect() { return sock_.connect(); }

  void run();

  void write(const std::string &message) { sock_.write(message); }

  void write_lines(const std::vector<std::string> &lines) {
    sock_.write_lines(lines);
  }

  Channel *get_channel(const std::string &channel);

  void add_channel(const std::string &channel);

  boost::optional<const std::vector<MessageCallback> &> callbacks_for_command(const std::string &command);

  const std::unordered_map<std::string, Channel> &channels() const { return channels_; }

  void join(const std::string &channel);
  void nick(const std::string &nick);

  bool channel_message(const std::string &channel, const std::string &message);

  Message read();
};
