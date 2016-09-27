#pragma once

#include <string>
#include <vector>

class MessagePrefix {
 private:
  std::string entity_;
  std::string user_;
  std::string host_;

 public:
  void set(const std::string &entity, const std::string &user, const std::string &host) {
    entity_ = entity;
    user_ = user;
    host_ = host;
  }

  const std::string &entity() const { return entity_; }
  const std::string &user() const { return user_; }
  const std::string &host() const { return host_; }
};

class Message {
 private:
  MessagePrefix prefix_;
  std::string command_;
  std::vector<std::string> params_;

  std::string line_;

 public:
  Message(const std::string &line);

  void add_param(const std::string &param) { params_.push_back(param); }

  const MessagePrefix &prefix() const { return prefix_; }
  const std::string &command() const { return command_; }
  const std::vector<std::string> &params() const { return params_; }

  const std::string &line() const { return line_; }
};
