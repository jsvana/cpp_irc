#pragma once

#include "lockless_queue.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/bind.hpp>

#include <memory>
#include <string>

class IrcSocket {
 private:
  std::string host_;
  std::string port_;
  lockless_queue<std::string> read_q_;
  // TODO(jsvana): actually use this
  bool ssl_;

  boost::asio::streambuf response_;

  boost::asio::io_service io_service_;

  std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_;

  const std::string LINE_SEP = "\r\n";

 public:
  IrcSocket(const std::string &host, const std::string &port, bool ssl = false) : host_(host), port_(port), ssl_(ssl) {
  }

  bool connect();

  void run();

  void write(const std::string &message);

  void write_lines(const std::vector<std::string> &lines);

  void read(const boost::system::error_code &error, std::size_t);

  void close();

  lockless_queue<std::string> &read_queue() {
    return read_q_;
  }
};
