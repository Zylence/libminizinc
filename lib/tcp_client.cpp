#include <minizinc/tcp_client.hh>
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace MiniZinc {

TCPClient::TCPClient(const std::string& host, int port)
    : std::ostream(&streamBuffer), host(host), port(port), socket_fd(-1), streamBuffer(*this) {
#ifdef _WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

TCPClient::~TCPClient() {
  if (socket_fd != -1) {
#ifdef _WIN32
    closesocket(socket_fd);
    WSACleanup();
#else
    close(socket_fd);
#endif
  }
}

bool TCPClient::connect() {
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    std::cerr << "Error creating socket" << std::endl;
    return false;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);

  if (::connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    std::cerr << "Error connecting to server" << std::endl;
    return false;
  }

  return true;
}


void TCPClient::send(const std::string& message) {
  ::send(socket_fd, message.c_str(), message.size(), 0);
}

std::string TCPClient::receive() {
  char buffer[1024];
  int len = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
  if (len > 0) {
    buffer[len] = '\0';  // Null-terminate the string
    return std::string(buffer);
  }
  return "";
}
}  // namespace MiniZinc