#include "socket.h"
#include <iostream>
#include <sys/time.h>

using namespace std;


Socket::Socket() {
  sock = -1;
  memset(&sock_addr, 0, sizeof(sock_addr));
}

Socket::~Socket() {
  if ( is_valid()) close(sock); //unistd.h
}

bool Socket::create() {
  sock = socket(AF_INET, SOCK_STREAM, 0);

  if (!is_valid()) {
    cout << "Fail to create socket: Socket already created\n";
    return false;
  }
  /*
  int reuse = 1;
  struct timeval timeout;
  timeout.tv_sec = 3;
  timeout.tv_usec = 0;
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
    cout <<"Fail to setSockOption\n";
    return false;
  }
  */
  return true;
}

bool Socket::bind (const int port) {
  if (!is_valid()) return false;

  sock_addr.sin_family = AF_INET; //ipv4
  sock_addr.sin_addr.s_addr = INADDR_ANY; //use available ip addr of lan card
  sock_addr.sin_port = htons(port); //convert to network byte order(big endian)

  if (::bind(sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1 ) { //::bind -> top level bind, not the name of this class!
    cout << "Fail to bind\n";
    return false;
  }
      
  return true;
}

bool Socket::listen() {
  if (!is_valid()) return false;
  if (::listen(sock, MAX_CONNECTIONS) == -1) {
    cout << "Fail to Listen\n" ;
    return false;
  }
  return true;
}

bool Socket::accept(Socket& new_socket) { // new_socket: to be used as path of both send&recv 
  int addr_len = sizeof(sock_addr);
  new_socket.sock = ::accept(sock, (struct sockaddr *)&sock_addr, (socklen_t *)&addr_len);

  if (new_socket.sock == -1) {
    cout << "Fail to accept client connection\n";
    return false;
  }
  cout << "accept..." << endl;
  return true;
}

//new_socket.send, new_socket.recv. by one same socket!
bool Socket::send(const string content) {
  if (::send(sock,content.c_str(), content.size(), 0) == -1) return false;
  else return true;
}

int Socket::recv(string& buffer) {


  char rcv_buf[PACKET_SIZE+2];
  memset(rcv_buf, 0, PACKET_SIZE+2);
  cout << "sock: " << sock << endl;
  int rcv_len;
  if ( (rcv_len = ::recv(sock, rcv_buf, PACKET_SIZE+2, 0)) == -1) {
    cout << "Fail to receive msg. errno:" << strerror(errno) << "\n";
    return -1;
  } else {
    buffer = rcv_buf;
    return rcv_len;
  }
}


ServerSocket::ServerSocket(int port) {
  if (!Socket::create())
    throw SocketException("Failed to create server socket\n"); //making exception class and return when (catch)
  if (!Socket::bind(port))
    throw SocketException("Failed to bind to port\n");
  if (!Socket::listen())
    throw SocketException("Failed to listen to socket\n");
}

ServerSocket::~ServerSocket() {}

void ServerSocket::accept(ServerSocket& sock) {
  if (!Socket::accept(sock)) throw SocketException("Failed to accept new socket\n");
}

int ServerSocket::recv(string& s) {
  int len;
  if ((len = Socket::recv(s)) == -1) {
    throw SocketException("Failed to recv\n");
  }
  return len;
}

void ServerSocket::send(string s) {
  if (!Socket::send(s)) throw SocketException("Failed to send\n");
}

