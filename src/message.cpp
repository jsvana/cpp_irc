#include "message.h"

Message::Message(const std::string &line) : line_(line) {
  std::size_t i = 0, end;

  if (line == "") {
    throw "Cannot parse empty line";
  }

  // Prefix format:
  // :entity!user@host
  // user and host are each optional

  // If there's a prefix, parse it
  if (line[i] == ':') {
    end = std::min(line.find(' ', i), line.find('!', i));
    auto entity = line.substr(i + 1, end - i - 1);
    std::string user = "";
    std::string host = "";

    i = end + 1;
    // Parse user
    if (line[i - 1] == '!') {
      end = std::min(line.find('@', i), line.find(' ', i));
      user = line.substr(i, end - i);
      i = end + 1;
    }

    // Parse host
    if (line[i - 1] == '@') {
      end = std::min(line.find('@', i), line.find(' ', i));
      host = line.substr(i, end - i);
      i = end + 1;
    }

    prefix_.set(entity, user, host);
  }

  end = line.find(' ', i);
  command_ = line.substr(i, end - i);
  i = end + 1;

  while (i < line.length()) {
    // Last param
    if (line[i] == ':') {
      params_.push_back(line.substr(i + 1));
      break;
    }

    end = line.find(' ', i);
    params_.push_back(line.substr(i, end - i));
    i = end + 1;
  }
}
