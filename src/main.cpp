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

  return 0;
}
