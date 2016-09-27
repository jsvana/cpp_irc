#pragma once

#include <string>
#include <unordered_map>

enum class UserMode : char {
  OWNER = 'q',
  ADMIN = 'a',
  OP = 'o',
  HALFOP = 'h',
  VOICE = 'v',
  UNKNOWN = '\0',
};

class User {
 public:
  std::string nick;
  UserMode mode;
};

class Channel {
 private:
  std::string name_;

  std::string topic_;
  std::unordered_map<std::string, User> users_;

 public:
  explicit Channel(const std::string &name) : name_(name) {}

  const std::string &name() const { return name_; }
  const std::string &topic() const { return topic_; }
  const std::unordered_map<std::string, User> &users() const { return users_; }

  void set_topic(const std::string &topic) { topic_ = topic; }
  void add_user(const std::string &nick, UserMode mode) {
    users_[nick].nick = nick;
    users_[nick].mode = mode;
  }
  void add_user(const std::string &nick) {
    add_user(nick, UserMode::UNKNOWN);
  }
};
