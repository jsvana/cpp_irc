#pragma once

#include "irc_socket.h"
#include "message.h"

#include <iostream>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <vector>

class Client {
 private:
  std::unique_ptr<IrcSocket> sock_;
  std::map<std::string, std::vector<std::function<void(Client &, const Message &)>>> callbacks_;

  const std::vector<std::function<void(Client &, const Message &)>> EMPTY_CALLBACKS;

 public:
  std::queue<std::string> nick_choices;

  Client(const std::string &host, const std::string &port);

  IrcSocket &socket() { return *sock_; }

  void set_nick_choices(const std::vector<std::string> &choices) {
    for (const auto &choice : choices) {
      nick_choices.push(choice);
    }
  }

  void add_callback(const std::string &command, const std::function<void(Client &, const Message &)> callback);

  bool connect() { return sock_->connect(); }

  void run();

  void write(const std::string &message) { sock_->write(message); }

  void write_lines(const std::vector<std::string> &lines) {
    sock_->write_lines(lines);
  }

  const std::vector<std::function<void(Client &, const Message &)>> &callbacks_for_command(const std::string &command);

  Message read() {
    auto line = sock_->read_queue().pop();
    std::cout << "RECV: " << line << std::endl;
    return Message(line);
  }
};
