#include "irc_socket.h"

#include <iostream>

bool IrcSocket::connect() {
  boost::asio::ip::tcp::resolver resolver(io_service_);
  boost::asio::ip::tcp::resolver::query query(host_, port_);
  auto iterator = resolver.resolve(query);

  boost::asio::ssl::context ctx(io_service_, boost::asio::ssl::context::sslv23);

  // No verification!
  ctx.set_verify_mode(boost::asio::ssl::context::verify_none);

  socket_ = std::make_unique<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(io_service_, ctx);
  auto &socket = socket_->lowest_layer();

  boost::asio::ip::tcp::resolver::iterator end;
  boost::system::error_code error = boost::asio::error::host_not_found;
  while (error && iterator != end) {
    socket.close();
    socket.connect(*iterator++, error);
  }

  if (error) {
    return false;
  }

  socket_->handshake(boost::asio::ssl::stream_base::client);

  return true;
}

void IrcSocket::run() {
  boost::asio::async_read_until(*socket_, response_, LINE_SEP, boost::bind(&IrcSocket::read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

  io_service_.run();
}

void IrcSocket::write(const std::string &message) {
  std::cout << "WRITE " << message << std::endl;
  boost::asio::write(*socket_, boost::asio::buffer(message + LINE_SEP));
}

void IrcSocket::write_lines(const std::vector<std::string> &lines) {
  for (const auto &line : lines) {
    write(line);
  }
}

void IrcSocket::read(const boost::system::error_code &error, std::size_t) {
  if (error) {
    close();
    return;
  }

  std::istream response_stream(&response_);
  std::string line;
  while (std::getline(response_stream, line)) {
    read_q_.push(line);
  }
  boost::asio::async_read_until(*socket_, response_, "\r\n", boost::bind(&IrcSocket::read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void IrcSocket::close() {
  socket_->lowest_layer().close();
  io_service_.stop();
}
