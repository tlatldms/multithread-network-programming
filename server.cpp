#include <iostream>
#include <string>
#include <string.h>
#include <thread>
#include <mutex>
#include "socket.h"

using namespace std;

//mutex mtx;

char all_packet[256]="HTTP/1.1 200 OK\r\nContent-Type: */*\r\nConnection: keep-alive\r\nKeep-Alive: timeout=30, max=200\r\nConnection: keep-alive\r\nKeep-Alive: timeout=300, max=200\r\n\r\n";
//string html_packet = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\nConnection: keep-alive\r\nKeep-Alive: timeout=30, max=200\r\n\r\n";

char* get_type(char *name) {
  int i, flag=0, j=0;
  char* type= (char*)malloc(sizeof(char)*7);
  memset(type, 0, 7);
  for (i=0;i<strlen(name); i++) {
    if (flag == 1) type[j++]=name[i]; 
    if (name[i]== '.') flag=1;
  }
  type[j] = '\0';
  return type;
}

char* get_filename(char* msg) {
  char * parsed_name = (char *)malloc(sizeof(char)*50);
  memset(parsed_name, 0, 50);
  int idx=0;
  for(int i=5; i<56; i++) {
    if (msg[i] == ' ') break;
    parsed_name[idx++] = msg[i];
  }
  parsed_name[idx] = '\0';
  return parsed_name;
}

void fun(ServerSocket * _sock) {
  int sock = (*_sock).get_sock();
  thread::id this_id = std::this_thread::get_id();
  cout << "쓰레드 번호: " << this_id << endl;

  char rcv_buf[PACKET_SIZE+2];
  memset(rcv_buf, 0, sizeof(rcv_buf));
  int rcv_len;

  if ( (rcv_len = ::recv( sock,  rcv_buf, PACKET_SIZE, 0)) == -1) {
    cout << "Fail to receive msg. errno:" << strerror(errno) << "\n";
  } else {
    cout << rcv_buf << endl;
    
    if (::send(sock, all_packet, strlen(all_packet), 0) == -1) {
       cout << "failed to send!" << endl;
    }
    cout << "헤더  패킷 보냈음! " << endl;

    if (rcv_buf[5] == ' ') { //첫 페이지
      char ind[12]="index.html\0";
      if (!(_sock->send_file(ind)))
          cout << errno << endl;
    } else { // 파일 요청
      if (!(_sock->send_requested_file(rcv_buf))) {
        cout << errno << endl;
      }
    }
    memset(rcv_buf, 0, sizeof(rcv_buf));
  }
  close(sock);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage : %s <port>\n", argv[0]);
    return -1;
  }
  int _port = atoi(argv[1]);
  ServerSocket server_sock(_port);
  try {
  while (true) {
    ServerSocket new_sock; // path to recv/send
    server_sock.accept(new_sock);

    thread t(fun, &new_sock);
    t.join();
  }
  } catch (SocketException& e) {
    cout << "Exception was caught: " << e.description() <<"\n Exit Program\n";
  }
  close(server_sock.get_sock());
  pthread_exit(NULL);
  return 0;
}
