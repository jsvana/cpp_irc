#include "channel.h"

void Channel::add_nicks(const std::set<std::string> &nicks) {
  for (const auto &nick : nicks) {
    nicks_.insert(nick);
  }
}
