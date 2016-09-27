#include "client.h"

#include <algorithm>
#include <iostream>

void dispatcher(Client &client) {
  client.write_lines({
    "USER jsvana 0.0.0.0 0.0.0.0 :jsvana test",
    "NICK " + client.nick_choices.front(),
  });

  while (true) {
    auto message = client.read();

    const auto &callbacks = client.callbacks_for_command(message.command());
    for (const auto &callback : callbacks) {
      callback(client, message);
    }
  }
}

Client::Client(const std::string &host, const std::string &port) {
  sock_ = std::make_unique<IrcSocket>(host, port);
}

void Client::add_callback(const std::string &command, const std::function<void(Client &, const Message &)> callback) {
  auto iter = callbacks_.find(command);
  if (iter == callbacks_.end()) {
    callbacks_.emplace(std::make_pair(command, std::vector<std::function<void(Client &, const Message &)>>()));
  }
  callbacks_[command].push_back(callback);
}

const std::vector<std::function<void(Client &, const Message &)>> &Client::callbacks_for_command(const std::string &command) {
  auto iter = callbacks_.find(command);
  if (iter == callbacks_.end()) {
    return EMPTY_CALLBACKS;
  }

  return iter->second;
}

void Client::setup_callbacks() {
  // Nick in use
  add_callback("433", [](Client &client, const Message &) {
    if (client.nick_choices.empty()) {
      throw "Out of nick choices";
    }
    client.nick_choices.pop();
    client.write("NICK " + client.nick_choices.front());
  });

  add_callback("PING", [](Client &client, const Message &message) {
    client.write("PONG " + message.params()[0]);
  });

  add_callback("JOIN", [](Client &client, const Message &message) {
    if (message.prefix().entity() == client.nick()) {
      client.add_channel(message.params()[0]);
      std::cout << "Active channels:";
      for (const auto &chan : client.channels()) {
        std::cout << " " << chan;
      }
      std::cout << std::endl;
    }
  });
}

void Client::run() {
  setup_callbacks();

  std::thread dispatcher_thread(dispatcher, std::ref(*this));

  sock_->run();

  dispatcher_thread.join();
}

Message Client::read() {
  // Read in batches because not all lines will end in \r\n
  std::size_t end;
  std::string line;
  do {
    end = unfinished_line_.find("\r\n");
    if (end == std::string::npos) {
      unfinished_line_ += sock_->read_queue().pop();
    }
  } while (end == std::string::npos);

  line = unfinished_line_.substr(0, end);
  unfinished_line_ = unfinished_line_.substr(end + 2);

  std::cout << "RECV " << line << std::endl;

  return Message(line);
}
