#pragma once

#include <set>
#include <string>

class Channel {
 private:
  std::string name_;

  std::string topic_;
  std::set<std::string> nicks_;

 public:
  Channel(const std::string &name) : name_(name) {}

  const std::string &name() const { return name_; }
  const std::string &topic() const { return topic_; }
  const std::set<std::string> &nicks() const { return nicks_; }

  void set_topic(const std::string &topic) { topic_ = topic; }
  void add_nicks(const std::set<std::string> &nicks);
};
