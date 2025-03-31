#pragma once
#include <string>
#include <iostream>


#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#undef ERROR
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace MiniZinc {
class TCPClient : public std::ostream {
public:
  TCPClient(const std::string& host = "127.0.0.1", int port = 4242);
  ~TCPClient();
  bool connect();
  std::string receive();

private:
  std::string host;
  int port;
  int socket_fd;  // For both Windows and POSIX

  // Custom stream buffer for TCP
  class TCPStreamBuffer : public std::streambuf {
  public:
    TCPStreamBuffer(TCPClient& client) : client(client) {}

  protected:
    int overflow(int c) override {
      if (c != EOF) {
        std::string message(1, static_cast<char>(c));
        client.send(message);
      }
      return c;
    }

  private:
    TCPClient& client;
  };

  TCPStreamBuffer streamBuffer;           // Stream buffer instance
  void send(const std::string& message);  // Send method
};
}  // namespace MiniZinc