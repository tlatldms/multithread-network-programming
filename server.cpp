#include <iostream>
#include <string>
#include <string.h>
#include <thread>
#include <mutex>
#include "socket.h"

using namespace std;

mutex mtx;

char all_packet[256]="HTTP/1.1 200 OK\r\nContent-Type: */*\r\nConnection: keep-alive\r\nKeep-Alive: timeout=30, max=200\r\nConnection: keep-alive\r\nKeep-Alive: timeout=300, max=200\r\n\r\n";
string html_packet = "HTTP/1.1 200 OK\r\nContent-Type:text/html\r\nConnection: keep-alive\r\nKeep-Alive: timeout=30, max=200\r\n\r\n";
string video_packet="HTTP/1.1 200 OK\r\nContent-Type: video/*\r\nConnection: keep-alive\r\nKeep-Alive: timeout=30, max=200\r\n\r\n";

int file_exists(char* filename) {
	FILE* file;
	if (file = fopen(filename, "rb")) {
		fclose(file);
		return 1;
	}
	return -1;
}
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

void fun(int sock) {
    cout <<"at first: " <<  sock<< endl; 
    cout <<"at second " << sock << endl;
    thread::id this_id = std::this_thread::get_id();
    cout << "쓰레드 번호: " << this_id << endl;

        /*
          cout <<"sock getsock: " <<sock.get_sock() << endl;
          string data = "";
          string& ref = data;
          cout << "len: " << sock.recv(ref) << "\n";
          cout << "msg from client: " << data << "\n";
          */
          
          char rcv_buf[PACKET_SIZE+2];
          memset(rcv_buf, 0, sizeof(rcv_buf));
          int rcv_len;
           
    cout <<"at third " << sock << endl;
        if ( (rcv_len = ::recv( sock,  rcv_buf, PACKET_SIZE, 0)) == -1) {
          cout << "Fail to receive msg. errno:" << strerror(errno) << "\n";
        } else {
          cout << rcv_buf << endl;
           if (rcv_buf[5] == ' ') { //첫 페이지
            
            if (::send(sock, html_packet.c_str(), html_packet.size(), 0) == -1) {
              cout << "failed to send!" << endl;
            }
            // sock.send(html_packet);
            cout << "html 패킷 보냈음! " << endl; 
            //int fr= open("./video/out.m3u8", O_RDONLY);
            int fr= open("index.html", O_RDONLY);
            if (fr < 0) {
              puts("ERROR when sending index\n");
              exit(1);
            }
         char buf[PACKET_SIZE+2];
            memset(buf, 0, sizeof(buf));

            int len;
            while ((len = read(fr,buf,PACKET_SIZE)) > 0) {
              ::send(sock, buf, len,0);
            }
            close(fr);

          } else { // 파일 요청
            char * file_name = get_filename(rcv_buf);
        printf("\n\n파일 이름: ttttt%sttttt\n\n", file_name );
           char* type = get_type(file_name); 
        printf("\n\n파일 타입: %s\n\n",type );
        
        if (strcmp(type,"min.js.map") == 0 || strcmp(type, "ico")==0) return;
        
        send(sock, all_packet, strlen(all_packet), 0);

						printf("\n\nALL 패킷 보냈음\n\n");
           if (file_exists(file_name) == -1) {
            cout << "파일이 없음 \n" ;
           }
            int fr= open(file_name, O_RDONLY);
            if (fr < 0) {
              puts("ERROR when sending something\n");
              exit(1);
            }
            char buf[PACKET_SIZE+2];
            memset(buf, 0, sizeof(buf));

            int len;
            while ((len = read(fr,buf,PACKET_SIZE)) > 0) {
              ::send(sock, buf, len,0);
            }
            close(fr);
            memset(rcv_buf, 0, sizeof(rcv_buf));


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
  //try {
      while (true) {
      ServerSocket new_sock; // path to recv/send
      server_sock.accept(new_sock);
      cout << "new_sock in main: " << new_sock.get_sock() << endl;
      

      //pthread_create(&thread_id,0,&fun, (void*)new_sock );
      //pthread_detach(thread_id);
      thread t(fun, new_sock.get_sock());
      t.join();
    }
  //} catch (SocketException& e) {
  //  cout << "Exception was caught: " << e.description() <<"\n Exit Program\n";
  //}
  close(server_sock.get_sock());
  pthread_exit(NULL);
  return 0;
}
