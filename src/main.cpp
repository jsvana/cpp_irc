#include "irc_socket.h"
#include "lockless_queue.h"
#include "message.h"

#include <iostream>
#include <string>
#include <thread>

void dispatcher(IrcSocket &sock) {
  sock.write_lines({
    "USER jsvana 0.0.0.0 0.0.0.0 :jsvana test",
    "NICK jsvana",
  });

  auto read = sock.read_queue();
  while (true) {
    auto line = read->pop();
    std::cout << "parsing " << line << std::endl;
    Message message(line);

    std::cout << message.prefix().server() << std::endl;
    std::cout << message.command() << std::endl;
  }
}

int main(int argc, char* argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
      return 1;
    }

    lockless_queue<std::string> read;

    IrcSocket sock(argv[1], argv[2], &read);

    if (!sock.connect()) {
      std::cerr << "Error connecting to " << argv[1] << ":" << argv[2] << std::endl;
      return 1;
    } else {
      std::cout << "Connected to " << argv[1] << ":" << argv[2] << std::endl;
    }

    std::thread dispatcher_thread(dispatcher, std::ref(sock));

    sock.run();

    dispatcher_thread.join();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
