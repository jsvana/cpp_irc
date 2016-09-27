#pragma once

#include <map>
#include <string>

enum UserMode {
  MODE_OWNER,
  MODE_ADMIN,
  MODE_OP,
  MODE_HALFOP,
  MODE_VOICE,
  MODE_UNKNOWN,
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
  std::map<std::string, User> users_;

 public:
  Channel(const std::string &name) : name_(name) {}

  const std::string &name() const { return name_; }
  const std::string &topic() const { return topic_; }
  const std::map<std::string, User> &users() const { return users_; }

  void set_topic(const std::string &topic) { topic_ = topic; }
  void add_user(const std::string &nick, UserMode mode);
};
