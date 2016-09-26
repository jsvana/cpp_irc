#pragma once

#include <string>
#include <vector>

class MessagePrefix {
 private:
  std::string server_;
  std::string user_;
  std::string host_;

 public:
  void set(const std::string &server, const std::string &user, const std::string &host) {
    server_ = server;
    user_ = user;
    host_ = host;
  }

  const std::string &server() const { return server_; }
  const std::string &user() const { return user_; }
  const std::string &host() const { return host_; }
};

enum CommandType {
  COMMAND_NUMBER,
  COMMAND_STRING,
};

class Command {
 private:
  // TODO(jsvana): switch to std::variant<>
  CommandType type_;
  int command_number_;
  std::string command_str_;

 public:
  void set(const CommandType type, int command_number, const std::string &command_str) {
    type_ = type;
    command_number_ = command_number;
    command_str_ = command_str;
  }

  const CommandType &type() const { return type_; }
  const int &number() const { return command_number_; }
  const std::string &string() const { return command_str_; }
};

class Message {
 private:
  MessagePrefix prefix_;
  Command command_;
  std::vector<std::string> params_;

 public:
  Message(const std::string &line);

  void add_param(const std::string &param) { params_.push_back(param); }

  const MessagePrefix &prefix() const { return prefix_; }
  const Command &command() const { return command_; }
  const std::vector<std::string> &params() const { return params_; }
};
