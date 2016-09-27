#include "channel.h"

void Channel::add_user(const std::string &nick, UserMode mode) {
  users_[nick].nick = nick;
  users_[nick].mode = mode;
}
