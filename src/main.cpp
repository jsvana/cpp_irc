#include "client.h"

#include <iostream>

int main(int argc, char* argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
      return 1;
    }

    Client client(argv[1], argv[2]);
    client.set_nick_choices({"jsvana", "jsvana_", "jsvana__"});

    if (!client.connect()) {
      std::cerr << "Error connecting to " << argv[1] << ":" << argv[2] << std::endl;
      return 1;
    } else {
      std::cout << "Connected to " << argv[1] << ":" << argv[2] << std::endl;
    }

    client.run();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
