#include "bot.h"

#include "channel.h"
#include "message.h"

#include <chaiscript/chaiscript.hpp>
//#include "chaiscript/extras/string_methods.hpp"

#include <iostream>
#include <memory>

std::tuple<std::string, std::vector<std::string>>
Command::parse_command(const Message &message) {
  auto last_arg = message.params[1];
  auto i = last_arg.find(' ');
  std::string cmd;
  if (i == std::string::npos) {
    cmd = last_arg.substr(1);
    i = last_arg.length();
  } else {
    cmd = last_arg.substr(1, i - 1);
  }

  i++;

  std::vector<std::string> args;
  while (i < last_arg.length()) {
    auto end = last_arg.find(' ', i);
    if (end == std::string::npos) {
      args.push_back(last_arg.substr(i));
      break;
    }
    args.push_back(last_arg.substr(i, end - i));
    i = end + 1;
  }

  return std::make_tuple(cmd, args);
}

namespace bot {

typedef std::function<void(const Message &, const Command &)> CommandCallback;
std::unordered_map<std::string, std::vector<CommandCallback>> callbacks_;

std::unique_ptr<Client> client_;

char command_prefix_ = '~';

boost::optional<std::string> nick_ = boost::none;

chaiscript::ChaiScript chai_;

boost::optional<const std::vector<CommandCallback> &>
callbacks_for_command(const std::string &command) {
  const auto iter = callbacks_.find(command);
  if (iter == callbacks_.end()) {
    return boost::none;
  }

  return iter->second;
}

void nick(const std::string &nick) {
  nick_ = nick;
  client_->nick(nick);
}
void join(const std::string &channel) { client_->join(channel); }
void write(const std::string &message) { client_->write(message); }

bool channel_message(const std::string &channel, const std::string &message) {
  return client_->channel_message(channel, message);
}

void add_callback(const std::string &command, const MessageCallback callback) {
  client_->add_callback(command, callback);
}

void add_command_callback(const std::string &command,
                          const CommandCallback callback) {
  callbacks_[command].push_back(callback);
}

bool has_channel(const std::string &channel) {
  return client_->has_channel(channel);
}

Channel *find_channel(const std::string &channel) {
  return client_->find_channel(channel);
}

void init_chaiscript() {
  // chai_.add(chaiscript::extras::string_methods::bootstrap());

  // This allows us to use vectors of strings properly in
  // scripts.
  chai_.add(chaiscript::bootstrap::standard_library::vector_type<
            std::vector<std::string>>("StringVec"));
  chai_.add(chaiscript::bootstrap::standard_library::map_type<
            std::map<std::string, User>>("UserMap"));

  chai_.add(chaiscript::fun(&nick), "nick");
  chai_.add(chaiscript::fun(&join), "join");
  chai_.add(chaiscript::fun(&write), "write");
  chai_.add(chaiscript::fun(&channel_message), "channel_message");
  chai_.add(chaiscript::fun(&add_callback), "add_callback");
  chai_.add(chaiscript::fun(&add_command_callback), "add_command_callback");
  chai_.add(chaiscript::fun(&has_channel), "has_channel");
  chai_.add(chaiscript::fun(&find_channel), "find_channel");

  chai_.add(chaiscript::user_type<Message>(), "Message");
  chai_.add(chaiscript::fun(&Message::command), "command");
  chai_.add(chaiscript::fun(&Message::params), "params");

  chai_.add(chaiscript::user_type<Command>(), "Command");
  chai_.add(chaiscript::fun(&Command::origin), "origin");
  chai_.add(chaiscript::fun(&Command::command), "command");

  chai_.add(chaiscript::user_type<Channel>(), "Channel");
  chai_.add(chaiscript::fun(&Channel::name), "name");
  chai_.add(chaiscript::fun(&Channel::topic), "topic");
  chai_.add(chaiscript::fun(&Channel::users), "users");

  chai_.add(chaiscript::user_type<User>(), "User");
  chai_.add(chaiscript::fun(&User::nick), "nick");
  // TODO(jsvana): add modes
}

void init_callbacks() {
  client_->add_callback("433",
                        [](const Message &message) { nick_ = boost::none; });

  client_->add_callback("PRIVMSG", [](const Message &message) {
    if (message.params.size() < 2) {
      std::cerr << "[BADC] " << message.line << std::endl;
      return;
    }

    if (message.params[1][0] != command_prefix_) {
      return;
    }

    std::string origin;
    if (message.params[0] == nick_) {
      origin = *message.user;
    } else {
      origin = message.params[0];
    }

    Command command(message, origin);

    const auto &callbacks = callbacks_for_command(command.command);
    if (!callbacks) {
      std::cout << "NO CALLBACKS FOR " << command.command << std::endl;
      return;
    }

    for (const auto &callback : *callbacks) {
      callback(message, command);
    }
  });
}

int run(const std::string &host, const std::string &port, char command_prefix,
        const std::string &script_path) {
  client_ = std::make_unique<Client>(host, port);
  command_prefix_ = command_prefix;

  init_chaiscript();
  init_callbacks();

  chai_.eval_file(script_path);

  chai_.eval<std::function<void()>>("init")();

  if (!client_->connect()) {
    std::cerr << "Error connecting to " << host << ":" << port << std::endl;
    return 1;
  } else {
    std::cout << "Connected to " << host << ":" << port << std::endl;
  }

  chai_.eval<std::function<void()>>("post_connect")();

  client_->run();

  return 0;
}

} // namespace bot
