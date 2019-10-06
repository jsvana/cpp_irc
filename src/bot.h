#pragma once

#include "client.h"

#include <functional>
#include <queue>
#include <string>
#include <tuple>
#include <vector>

class Command {
private:
  const std::tuple<std::string, std::vector<std::string>> command_parts_;
  std::tuple<std::string, std::vector<std::string>>
  parse_command(const Message &message);

public:
  const std::string origin;
  const std::string command;
  const std::vector<std::string> args;

  Command(const Message &message, const std::string &orig)
      : command_parts_(parse_command(message)), origin(orig),
        command(std::get<0>(command_parts_)),
        args(std::get<1>(command_parts_)) {}
};

namespace bot {
int run(const std::string &host, const std::string &port, char command_prefix,
        const std::string &script_path);
}
