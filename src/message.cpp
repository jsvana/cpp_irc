#include "message.h"

#include <stdexcept>

const std::tuple<boost::optional<std::string>, boost::optional<std::string>, boost::optional<std::string>, std::string, std::vector<std::string>> Message::parse_line(const std::string& line) {
  std::size_t i = 0, end;

  if (line == "") {
    throw std::invalid_argument("Cannot parse empty line");
  }

  // Prefix format:
  // :entity!user@host
  // user and host are each optional

  boost::optional<std::string> entity, user, host;

  // If there's a prefix, parse it
  if (line[i] == ':') {
    end = std::min(line.find(' ', i), line.find('!', i));
    entity = line.substr(i + 1, end - i - 1);

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
  }

  end = line.find(' ', i);
  auto command = line.substr(i, end - i);
  i = end + 1;

  std::vector<std::string> parsed_params;

  while (i < line.length()) {
    // Last param
    if (line[i] == ':') {
      parsed_params.push_back(line.substr(i + 1));
      break;
    }

    end = line.find(' ', i);
    parsed_params.push_back(line.substr(i, end - i));
    i = end + 1;
  }

  return std::make_tuple(entity, user, host, command, parsed_params);
}
