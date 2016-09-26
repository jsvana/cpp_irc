#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/bind.hpp>

#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

const int IRC_BUFFER_SIZE = 512;

template<typename T>
class lockless_queue {
 private:
  std::mutex q_lock_;
  std::condition_variable cv_;
  std::queue<T> q_;

 public:
  void push(T value) {
    std::lock_guard<std::mutex> guard(q_lock_);
    cv_.notify_all();
    q_.push(value);
  }

  T pop() {
    std::unique_lock<std::mutex> lock(q_lock_);
    cv_.wait(lock, [this]{ return !q_.empty(); });
    auto ret = q_.front();
    q_.pop();
    return ret;
  }

  bool empty() {
    std::lock_guard<std::mutex> guard(q_lock_);
    return q_.empty();
  }
};

class irc_socket {
 private:
  std::string host_;
  std::string port_;
  lockless_queue<std::string> *read_q_;
  // TODO(jsvana): actually use this
  bool ssl_;

  boost::asio::streambuf response_;

  boost::asio::io_service io_service_;

  std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_;

  const std::string LINE_SEP = "\r\n";

 public:
  irc_socket(const std::string &host, const std::string &port, lockless_queue<std::string> *read_q, bool ssl = false) : host_(host), port_(port), read_q_(read_q), ssl_(ssl) {
  }

  bool connect() {
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

  void run() {
    boost::asio::async_read_until(*socket_, response_, LINE_SEP, boost::bind(&irc_socket::read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

    io_service_.run();
  }

  void write(const std::string &message) {
    std::cout << "WRITE " << message << std::endl;
    boost::asio::write(*socket_, boost::asio::buffer(message + LINE_SEP));
  }

  void write_lines(const std::vector<std::string> &lines) {
    for (const auto &line : lines) {
      write(line);
    }
  }

  void read(const boost::system::error_code &error, std::size_t) {
    if (error) {
      close();
      return;
    }

    std::istream response_stream(&response_);
    std::string line;
    while (std::getline(response_stream, line)) {
      read_q_->push(line);
    }
    boost::asio::async_read_until(*socket_, response_, "\r\n", boost::bind(&irc_socket::read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
  }

  void close() {
    socket_->lowest_layer().close();
    io_service_.stop();
  }

  lockless_queue<std::string> *read_queue() {
    return read_q_;
  }
};

void dispatcher(irc_socket &sock) {
  sock.write_lines({
    "USER jsvana 0.0.0.0 0.0.0.0 :jsvana test",
    "NICK jsvana",
  });

  auto read = sock.read_queue();
  while (true) {
    auto line = read->pop();
    std::cout << line << std::endl;
  }
}

int main(int argc, char* argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
      return 1;
    }

    lockless_queue<std::string> read;

    irc_socket sock(argv[1], argv[2], &read);

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
