#include <sys/socket.h>
#include <netinet/in.h>

namespace net {

template<size_t BUFFER_SIZE>
class TcpSocket {
 private:
  int addr_;
  int port_;
  int fd_;

 public:
  TcpSocket(int addr, int port) : addr_(addr), port_(port);

  int read(std::string &data);
  int write(const std::string &data);
};

class TcpServer {
};

} // namespace net
