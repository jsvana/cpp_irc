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

    // Nick in use
    client.add_callback("433", [](Client &client, const Message &) {
      if (client.nick_choices.empty()) {
        throw "Out of nick choices";
      }
      client.write("NICK " + client.nick_choices.front());
      client.nick_choices.pop();
    });

    client.add_callback("PING", [](Client &client, const Message &message) {
      client.write("PONG " + message.params()[0]);
    });

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
