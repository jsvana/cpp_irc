#pragma once

#include "irc_socket.h"
#include "message.h"

#include <memory>
#include <string>
#include <thread>

class Client {
 private:
  std::unique_ptr<IrcSocket> sock_;

 public:
  Client(const std::string &host, const std::string &port);

  IrcSocket &socket() { return *sock_; }

  bool connect() { return sock_->connect(); }

  void run();

  void write(const std::string &message) { sock_->write(message); }

  void write_lines(const std::vector<std::string> &lines) {
    sock_->write_lines(lines);
  }

  Message read() {
    auto line = sock_->read_queue().pop();
    return Message(line);
  }
};
