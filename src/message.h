#pragma once

#include <boost/optional.hpp>

#include <string>
#include <tuple>
#include <vector>

class Message {
 private:

  const std::tuple<boost::optional<std::string>, boost::optional<std::string>, boost::optional<std::string>, std::string, std::vector<std::string>> pieces_;

  const std::tuple<boost::optional<std::string>, boost::optional<std::string>, boost::optional<std::string>, std::string, std::vector<std::string>> parse_line(const std::string& line);

 public:
  const std::string line;
  const boost::optional<std::string> entity;
  const boost::optional<std::string> user;
  const boost::optional<std::string> host;

  const std::string command;
  const std::vector<std::string> params;

  Message(const std::string &input_line)
    : pieces_(parse_line(input_line)),
      line(input_line),
      entity(std::get<0>(pieces_)),
      user(std::get<1>(pieces_)),
      host(std::get<2>(pieces_)),
      command(std::get<3>(pieces_)),
      params(std::get<4>(pieces_)) {}
};
