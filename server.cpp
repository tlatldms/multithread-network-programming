#include <iostream>
#include <string>
#include <string.h>



#include "socket.h"

using namespace std;

string html_packet = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\nConnection: keep-alive\r\nKeep-Alive: timeout=30, max=200\r\n\r\n";
string video_packet="HTTP/1.1 200 OK\r\nContent-Type: video/*\r\nConnection: keep-alive\r\nKeep-Alive: timeout=30, max=200\r\n\r\n";




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
          new_sock.send(video_packet);
          int fr= open("qq.m3u8", O_RDONLY);
          if (fr < 0) {
            puts("ERROR when sending index\n");
            exit(1);
          }
          char buf[PACKET_SIZE+2];
          memset(buf, 0, sizeof(buf));
          int len, cnt=0;
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
