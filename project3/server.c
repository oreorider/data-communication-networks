#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>

#define PORT 7777
#define BUFFER_SIZE 1000000

void forward (int sock);

int main( int argc, char *argv[] ) {
  int sockfd, newsockfd, portno = PORT;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  clilen = sizeof(cli_addr);

  /* First call to socket() function */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  
  // port reusable
  int tr = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }

  /* Initialize socket structure */
  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  
  /* Now bind the host address using bind() call.*/
  if ( bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1 ){
    perror("bind error");
    exit(1);
  }

  /* listen on socket you created */
  if ( listen(sockfd, 10) == -1 ){
    perror("listen error");
    exit(1);
  }

  printf("Server is running on port %d\n", portno);

  while (1) {
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    
    if ( newsockfd == -1 ){
      perror("accept error");
      exit(1);
    }
    forward(newsockfd);
  }

  return 0;
}

/* 
  TODO (1): Forward received HTTP request to the web server
      Hint 1:
          The HTTP request has a hostname in the path. 
          Take the IP address with the hostname and open a socket with the address.
          (use gethostbyname() function)
      
      Hint 2:
          You should modify the path in the request header before forwarding it to a web sever.
          ex) "GET http://snu.nxclab.org:9000/index.html HTTP/1.1" -> "GET /index.html HTTP/1.1"

  TODO (2): Receive responses from the web server and forward them to the browser
  TODO (3): Modify HTML before forwarding response message to browser.
            Replace 20xx-xxxxx to your student id (ex. 20xx-xxxxx --> 2012-34567)
            And change the order of image displayed in browser
      
 */
void forward(int sock) {
  int offset, bytes;
  char buffer[9000];
  bzero(buffer,9000);

  offset = 0;
  bytes = 0;
  do {
    bytes = recv(sock, buffer + offset, 1500, 0);
    offset += bytes;
    
    if (strncmp(buffer + offset - 4, "\r\n\r\n", 4) == 0) break;
  } while (bytes > 0);

  if (offset < 0) {
    printf("recv() error\n");
    return;
  } else if (bytes == 0) {
    printf("Client disconnected unexpectedly\n");
    return;
  }
  
  printf("-----------------------------------\n");
  printf("%s\n", buffer);
  if(strstr(buffer, "snu.nxclab.org")==NULL) return;//only process nxclab.org

  //struct hostent* hostentry = gethostbyname("snu.nxclab.org");
  
  int sockfd;

  char domain_name[100] = "snu.nxclab.org";//NEED TO MAKE THIS DYNAMIC? or is it good? idk

  struct addrinfo hints, *results;

  struct sockaddr_in serv_addr;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if(getaddrinfo(domain_name, "7777", &hints, &results) != 0){
    perror("domain name error");
    exit(1);
  }
  sockfd = socket(results->ai_family, results->ai_socktype, results->ai_protocol);

  connect(sockfd, results->ai_addr, results->ai_addrlen);
  printf("server connected\n");

  
  char* after_slash_ptr = strstr(buffer, "snu.nxclab.org") + 19;
  char* before_http_ptr = strstr(buffer, "HTTP/1.1")-1;
  printf("test\n");
  char details[100];
  int detail_length = before_http_ptr - after_slash_ptr;
  printf("detail length is %d\n", detail_length);

  int index = 0;
  while(after_slash_ptr != before_http_ptr){
    details[index] = *after_slash_ptr;
    after_slash_ptr+=1;
    index+=1;
  }
  details[detail_length] = '\0';
  printf("detail is %s\n", details);

  
  char* newline_ptr = strstr(buffer, "\n");
  char* buffer_end = strstr(buffer, "\r\n\r\n")+4;
  char remainder[9000];
  strncpy(remainder, newline_ptr, buffer_end-newline_ptr);
  buffer_end = '\0';

  char* messsage_to_send;
  asprintf(&messsage_to_send, "%s %s %s%s", "GET", details, "HTTP/1.1", remainder);
  printf("sending message\n\n%s\n", messsage_to_send);
  send(sockfd, messsage_to_send, strlen(messsage_to_send), 0);

  char receive_buffer[9000]

  do{
      bytes = recv(newsockfd, buf+offset, 1500, 0);
      offset += bytes;
      if (strncmp(buf + offset - 4, "\r\n\r\n", 4) == 0) break; //if end of http
    } while(bytes > 0);
    printf("message received\n");

  if(bytes < 0){
      perror("receive error");
      exit(1);
    }
    else if (bytes == 0){
      perror("client disconneced unexpectedly");
      exit(1);
    }

  /*

  bzero((char*) &serv_addr, sizeof(serv_addr));//memset
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = *((unsigned long *)hostentry->h_addr);

  serv_addr.sin_port = htons(7777);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }
  connect(sockfd, )
  //bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  */

  /* TODO (1): Forward received HTTP request to the web server */

  /* TODO (2): Receive responses from the web server and forward them to the browser */

  /* TODO (3): Modify HTML before forwarding response message to browser.
            Replace 20xx-xxxxx to your student id (ex. 20xx-xxxxx --> 2012-34567)
            And change the order of image displayed in browser */
}

