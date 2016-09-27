#pragma once

#include "irc_socket.h"
#include "message.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class Client {
 private:
  std::unique_ptr<IrcSocket> sock_;
  std::map<std::string, std::vector<std::function<void(Client &)>>> callbacks_;

  const std::vector<std::function<void(Client &)>> EMPTY_CALLBACKS;

 public:
  Client(const std::string &host, const std::string &port);

  IrcSocket &socket() { return *sock_; }

  void add_callback(const std::string &command, const std::function<void(Client &)> callback);

  bool connect() { return sock_->connect(); }

  void run();

  void write(const std::string &message) { sock_->write(message); }

  void write_lines(const std::vector<std::string> &lines) {
    sock_->write_lines(lines);
  }

  const std::vector<std::function<void(Client &)>> &callbacks_for_command(const std::string &command);

  Message read() {
    auto line = sock_->read_queue().pop();
    return Message(line);
  }
};
