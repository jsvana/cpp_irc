#include "client.h"

#include <iostream>

void dispatcher(Client &client) {
  client.write_lines({
    "USER jsvana 0.0.0.0 0.0.0.0 :jsvana test",
    "NICK jsvana",
  });

  while (true) {
    auto message = client.read();
    std::cout << message.prefix().server() << std::endl;
    std::cout << message.command() << std::endl;
  }
}

Client::Client(const std::string &host, const std::string &port) {
  sock_ = std::make_unique<IrcSocket>(host, port);
}

void Client::run() {
  std::thread dispatcher_thread(dispatcher, std::ref(*this));

  sock_->run();

  dispatcher_thread.join();
}
