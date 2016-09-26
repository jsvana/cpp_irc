#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/use_future.hpp>

#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

const int IRC_BUFFER_SIZE = 512;
const int TIMEOUT = 200;

template<typename T>
class lockless_queue {
 private:
  std::mutex q_lock_;
  std::condition_variable cv_;
  std::queue<T> q_;

 public:
  void push(T value) {
    std::lock_guard<std::mutex> guard(q_lock_);
    std::cout << "Adding value" << std::endl;
    cv_.notify_all();
    q_.push(value);
  }

  T pop() {
    std::unique_lock<std::mutex> lock(q_lock_);
    std::cout << "waiting" << std::endl;
    cv_.wait(lock, [this]{ return !q_.empty(); });
    std::cout << "done" << std::endl;
    auto ret = q_.front();
    std::cout << ret << std::endl;
    q_.pop();
    return ret;
  }

  bool empty() {
    std::lock_guard<std::mutex> guard(q_lock_);
    return q_.empty();
  }
};

class lockless_socket {
 private:
  std::mutex q_lock_;

  int timeout_;
  bool ssl_;
  std::string host_;
  std::string port_;

  std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_;

 public:
  lockless_socket(const std::string &host, const std::string &port, const int timeout) : host_(host), port_(port), timeout_(timeout) {
  }

  bool connect() {
    boost::asio::io_service io_service;

    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(host_, port_);
    auto iterator = resolver.resolve(query);

    boost::asio::ssl::context ctx(io_service, boost::asio::ssl::context::sslv23);

    // No verification!
    ctx.set_verify_mode(boost::asio::ssl::context::verify_none);

    socket_ = std::make_unique<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(io_service, ctx);
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

  std::size_t read_lines(std::vector<std::string> &lines) {
    std::lock_guard<std::mutex> guard(q_lock_);

    std::size_t size, total_size = 0;
		boost::asio::streambuf response;
    do {
      // Problem here is that we need to have a timeout on read. What should probably happen is we
      // should move all these synchronous operations to their async counterparts and then pass
      // the queues to the lockless_socket constructor.
      std::future<std::size_t> read_fut = boost::asio::async_read_until(*socket_, response, "\r\n", boost::asio::use_future);
      if (read_fut.wait_for(std::chrono::milliseconds(timeout_)) != std::future_status::timeout) {
        size = read_fut.get();
        total_size += size;

        std::istream response_stream(&response);
        std::string line;
        while (std::getline(response_stream, line)) {
          lines.push_back(line);
        }
      } else {
        std::cerr << "timeout" << std::endl;
        socket_->lowest_layer().cancel();
        return -1;
      }
    } while (size > 0);

    return total_size;
  }

  std::size_t write(const std::string &message) {
    std::lock_guard<std::mutex> guard(q_lock_);
    std::cout << "attempting write" << std::endl;
    return boost::asio::write(*socket_, boost::asio::buffer(message));
  }

  auto &get_io_service() {
    return socket_->get_io_service();
  }
};

void reader(lockless_socket &socket, lockless_queue<std::string> &read) {
  std::vector<std::string> lines;
  while (true) {
    lines.clear();
    socket.read_lines(lines);
    for (const auto &line : lines) {
      read.push(line);
    }
  }
}

void writer(lockless_socket &socket, lockless_queue<std::string> &write) {
  while (true) {
    auto line = write.pop();
    socket.write(line);
  }
}

void dispatcher(lockless_queue<std::string> &read, lockless_queue<std::string> &write) {
  std::string req = "USER jsvana 0.0.0.0 0.0.0.0 :jsvana test\r\n"
    "NICK jsvana\r\n";
  write.push(req);

  while (true) {
    auto line = read.pop();
    std::cout << line << std::endl;
  }
}

int main(int argc, char* argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
      return 1;
    }

    lockless_socket sock(argv[1], argv[2], TIMEOUT);

    std::thread thread([&sock](){ sock.get_io_service().run(); });

    if (!sock.connect()) {
      std::cerr << "Error connecting to " << argv[1] << ":" << argv[2] << std::endl;
      return 1;
    } else {
      std::cout << "Connected to " << argv[1] << ":" << argv[2] << std::endl;
    }

    lockless_queue<std::string> read, write;

    std::thread dispatcher_thread(dispatcher, std::ref(read), std::ref(write));
    std::thread reader_thread(reader, std::ref(sock), std::ref(read));
    std::thread writer_thread(writer, std::ref(sock), std::ref(write));

    reader_thread.join();
    writer_thread.join();
    dispatcher_thread.join();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
