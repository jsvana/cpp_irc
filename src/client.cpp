#include "client.h"

#include <iostream>

void dispatcher(Client &client) {
  client.write_lines({
    "USER jsvana 0.0.0.0 0.0.0.0 :jsvana test",
    "NICK jsvana",
  });

  while (true) {
    auto message = client.read();

    const auto &callbacks = client.callbacks_for_command(message.command());
    for (const auto &callback : callbacks) {
      callback(client);
    }
  }
}

Client::Client(const std::string &host, const std::string &port) {
  sock_ = std::make_unique<IrcSocket>(host, port);
}

void Client::add_callback(const std::string &command, const std::function<void(Client &)> callback) {
  auto iter = callbacks_.find(command);
  if (iter == callbacks_.end()) {
    callbacks_.emplace(std::make_pair(command, std::vector<std::function<void(Client &)>>()));
  }
  callbacks_[command].push_back(callback);
}

const std::vector<std::function<void(Client &)>> &Client::callbacks_for_command(const std::string &command) {
  auto iter = callbacks_.find(command);
  if (iter == callbacks_.end()) {
    return EMPTY_CALLBACKS;
  }

  return iter->second;
}

void Client::run() {
  std::thread dispatcher_thread(dispatcher, std::ref(*this));

  sock_->run();

  dispatcher_thread.join();
}
