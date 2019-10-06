#include "bot.h"

#include <iostream>
#include <thread>

const char COMMAND_PREFIX = '~';

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
    return 1;
  }

  bot::run(argv[1], argv[2], COMMAND_PREFIX, "../assets/main.chai");

  /*
  Bot bot(argv[1], argv[2], COMMAND_PREFIX);
  bot.set_nick_choices({"starkbot", "starkbot_", "starkbot__"});

  bot.add_starting_channels({"#encoded-test"});

  bot.add_callback("JOIN", [](const Message& message) {
    std::cout << "Joined channel " << message.params[0] << std::endl;
  });

  // Example command
  bot.add_command_callback("test", [](Bot* bot, const Message& message, const
  Command& command) { if (command.args.size() < 1) {
      bot->channel_message(command.origin, *message.user + ": Must provide an
  argument"); return;
    }

    auto arg = command.args[0];
    int start = 0, end = arg.length() - 1;
    while (start < end) {
      char tmp = arg[start];
      arg[start] = arg[end];
      arg[end] = tmp;
      start++;
      end--;
    }
    bot->channel_message(command.origin, *message.user + ": " + arg);
  });

  if (!bot.connect()) {
    std::cerr << "Error connecting to " << argv[1] << ":" << argv[2] <<
  std::endl; return 1; } else { std::cout << "Connected to " << argv[1] << ":"
  << argv[2] << std::endl;
  }

  bot.run();
  */

  return 0;
}
