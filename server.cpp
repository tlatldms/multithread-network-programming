#include <sys/socket.h>
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string>
#include <string.h>

#include <unistd.h>

using namespace std;

#define MAX_CONNECTIONS 10
#define PORT 5555
#define PACKET_SIZE 5000
string html_packet = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\nConnection: keep-alive\r\nKeep-Alive: timeout=30, max=200\r\n\r\n";
class SocketException {
  public:
    SocketException(string s) : err_string(s) {};
    ~SocketException() {};

    string description() {
      return err_string;
    }
  private:
    string err_string; 
};

class Socket {
  public:
    Socket();
    virtual ~Socket();

    bool create();
    bool bind(const int port);
    bool listen();
    bool accept(Socket&);
    
    //Data Transmission
    bool send(const string);
    int recv(string&);
    
    int get_sock() {
      return sock;
    }
    bool is_valid() {
      return sock != -1;
    }

  private:
    int sock;
    sockaddr_in sock_addr;
};

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

  int reuse = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
    cout <<"Fail to setSockOption\n";
    return false;
  }
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
  return true;
}

//new_socket.send, new_socket.recv. by one same socket!
bool Socket::send(const string content) {
  if (::send(sock,content.c_str(), content.size(), 0) == -1) return false;
  else return true;
}

int Socket::recv(string& buffer) {
  char rcv_buf[PACKET_SIZE+2];
  buffer="";
  memset(rcv_buf, 0, PACKET_SIZE+2);
  int rcv_len;
  if ( (rcv_len = ::recv(sock, rcv_buf, PACKET_SIZE, 0)) == -1) {
    cout << "Fail to receive msg. errno:" << errno << "\n";
    return -1;
  } else {
    buffer = rcv_buf;
    return rcv_len;
  }
}


class ServerSocket : private Socket {
  public:
    ServerSocket(){};
    ServerSocket(int port);
    virtual ~ServerSocket();
    int get_sock() {
      return Socket::get_sock();
    }
    int recv(string& s);
    void send(string s);
    void accept(ServerSocket&);
};

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

int main() {

  try {
    ServerSocket server_sock(PORT);
    while (true) {
      ServerSocket new_sock; // path to recv/send
      server_sock.accept(new_sock);

      try {
        while (true) {
          string data;
          cout << "len: " << new_sock.recv(data) << "\n";
          cout << "msg from client: " << data << "\n";
          new_sock.send(html_packet);
          int fr= open("index.html", O_RDONLY);
          if (fr < 0) {
            puts("ERROR when sending index\n");
            exit(1);
          }
          char buf[PACKET_SIZE+2];
          memset(buf, 0, sizeof(buf));
          int len;
          while ((len = read(fr,buf,PACKET_SIZE)) > 0) {
            send(new_sock.get_sock(), buf, len,0);
          }
          close(fr);
        }
      } catch (SocketException& e) {
      }
    }
  } catch (SocketException& e) {
    cout << "Exception was caught: " << e.description() <<"\n Exit Program\n";
  }
  return 0;
}
