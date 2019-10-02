#pragma once

#include "channel.h"
#include "irc_socket.h"
#include "queue.h"
#include "message.h"

#include <boost/optional.hpp>

#include <functional>
#include <iostream>
#include <list>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

typedef std::function<void(const Message&)> MessageCallback;

class Client {
 private:
  IrcSocket sock_;
  std::unordered_map<std::string, std::vector<MessageCallback>> callbacks_;

  queue<std::string> message_lines_;

  std::list<std::string> pending_joins_;

  std::unordered_map<std::string, Channel> channels_;
  std::unordered_map<char, UserMode> prefixes_;

  void setup_callbacks();

  std::string unfinished_line_;

 public:
  Client(const std::string& host, const std::string& port) : sock_(host, port) {
    setup_callbacks();
  }

  IrcSocket& socket() { return sock_; }

  void add_prefix_mapping(char prefix, UserMode mode) { prefixes_[prefix] = mode; }

  const std::unordered_map<char, UserMode>& prefixes() const { return prefixes_; }

  void add_message_line(const std::string& line) { message_lines_.push(line); }

  // TODO(jsvana): move to try_pop so we don't block the main thread
  const std::string pop_message_line() { return message_lines_.pop(); }

  void add_callback(const std::string &command, const MessageCallback callback);

  bool connect() { return sock_.connect(); }

  void run();

  void write(const std::string& message) {
    std::cout << "[SEND] " << message << std::endl;
    sock_.write(message);
  }

  void write_lines(const std::vector<std::string>& lines) {
    sock_.write_lines(lines);
  }

  Channel *get_channel(const std::string& channel);

  void add_channel(const std::string& channel);

  boost::optional<const std::vector<MessageCallback>&> callbacks_for_command(const std::string& command);

  const std::unordered_map<std::string, Channel>& channels() const { return channels_; }

  void join(const std::string& channel);
  void nick(const std::string& nick);

  bool channel_message(const std::string& channel, const std::string& message);

  Message read();
};
