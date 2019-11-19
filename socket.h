#ifndef socket_class
#define socket_class

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <cerrno>

using namespace std;



#define MAX_CONNECTIONS 10
#define PORT 5555
#define PACKET_SIZE 1024


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

    bool send_requested_file(char buffer[PACKET_SIZE]);
    bool send_file(char file_name[50]);
    int recv(string&);
    
    int get_sock() {
      return sock;
    }
    sockaddr_in get_addr() {
      return sock_addr;
    }
    bool is_valid() {
      return sock != -1;
    }

  
    int sock;
    sockaddr_in sock_addr;
};

class ServerSocket : private Socket {
  public:
    ServerSocket(){};
    ServerSocket(int port);
    virtual ~ServerSocket();
    int get_sock() {
      return Socket::get_sock();
    }

    sockaddr_in get_addr() {
      return Socket::get_addr();
    }

    int recv(string& s);
    void send(string s);
    bool send_requested_file(char buffer[PACKET_SIZE]);
    bool send_file(char file_name[50]);
    void accept(ServerSocket&);
};

#endif
