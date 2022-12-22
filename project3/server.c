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
void forward_2(int sock);

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
  char buffer[BUFFER_SIZE];
  bzero(buffer,BUFFER_SIZE);

  offset = 0;
  bytes = 0;
  do {
    bytes = recv(sock, buffer + offset, 1500, 0);
    offset += bytes;
    
    if (strncmp(buffer + offset - 4, "\r\n\r\n", 4) == 0) {
      printf("end found\n");
      break;
    }
  } while (bytes > 0);

  if (offset < 0) {
    printf("recv() error\n");
    return;
  } else if (bytes == 0) {
    printf("Client disconnected unexpectedly\n");
    return;
  }
  
  printf("-----------------------------------\n");
  printf("%s", buffer);
  printf("offset is %d bytes\n", offset);
  //for(int i=0; i<strlen(buffer); i++){
  // printf("%02x ", buffer[i]);
  //}
  printf("\n");

  if(strstr(buffer, "snu.nxclab.org")!=NULL){//if from phone to computer
    printf("phone asking computer for nxclab\n");
    int sockfd, sockconnection;
    char domain_name[100] = "snu.nxclab.org";//NEED TO MAKE THIS DYNAMIC? or is it good? idk
    struct addrinfo hints, *results;
    //struct sockaddr_in serv_addr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(domain_name, "9000", &hints, &results) != 0){
      perror("domain name error");
      exit(1);
    }

    struct sockaddr_in *sin = (struct sockaddr_in *) results->ai_addr;
    char* ip;
    
    printf("value of ip addr: %.*s\n", (int)results->ai_addrlen, inet_ntoa(sin->sin_addr));

    sockfd = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
    if(sockfd == -1){
      perror("socket error");
      exit(1);
    }

    printf("socket created\n");
    if(sockconnection = connect(sockfd, results->ai_addr, results->ai_addrlen) == -1){
      perror("connection failed");
      exit(1);
    }
    freeaddrinfo(results);
    printf("server connected\n");

    //string manipulation, remove domain, leave only the thing after / ex) /index.html
    char* after_slash_ptr = strstr(buffer, ":9000") + 5;
    char* before_http_ptr = strstr(buffer, "HTTP/1.1")-1;
    //int size_of_message = offset;
    char details[BUFFER_SIZE];
    //bzero(details, BUFFER_SIZE);
    int detail_length = before_http_ptr - after_slash_ptr;
    printf("detail length is %d\n", detail_length);

    int index = 0;
    while(after_slash_ptr != before_http_ptr){
      details[index] = *after_slash_ptr;
      after_slash_ptr+=1;
      index+=1;
    }
    details[detail_length] = '\0';
    printf("detail is >%s<\n", details);

    char* newline_ptr = strstr(buffer, "\r");
    //char* buffer_end = strstr(buffer, "\r\n\r\n")+4;
    char* buffer_end = buffer+offset;//pointer to the element next to the last 

    char remainder[BUFFER_SIZE];
    //bzero(remainder, BUFFER_SIZE);
    /*
    index=0;
    for(char* chptr = newline_ptr; chptr != buffer_end+1; chptr++){
      remainder[index] = *chptr;
      index+=1;
    }*/

    strncpy(remainder, newline_ptr, buffer_end-newline_ptr);
    buffer_end = "\0";
    //char message_to_send[BUFFER_SIZE];
    //bzero(message_to_send, BUFFER_SIZE);

    //strcat(message_to_send, "GET ");
    //strcat(message_to_send, details);
    //strcat(message_to_send, " HTTP/1.1");
    //strcat(message_to_send, remainder);

    char* message_to_send;
    asprintf(&message_to_send, "%s%s%s%s", "GET ", details, " HTTP/1.1", remainder);

    //buffer_end = "\0";
    //char* messsage_to_send;
    //asprintf(&message_to_send, "%s %s %s%s", "GET", details, "HTTP/1.1", remainder);
    //string parsing end  
    //for(int i=0; i<strlen(message_to_send); i++){
    //  printf("%02x ", message_to_send[i]);
    //}
    printf("\n");

    /*
    bytes = send(sockfd, buffer, strlen(message_to_send), 0);
    printf("sent bytes : %d\n\n", bytes);
    printf("=============message to send to nxcserver============\n\n%s", message_to_send);
    */
    int length = strlen(message_to_send);

    printf("length is %d\n", length);

    //send the message
    
    while(length > 0){
      bytes = send(sockfd, message_to_send, length, 0);
      printf("send bytes : %d\n", bytes);
      length = length - bytes;
    }
    //bytes = send(sockfd, message_to_send, length, 0);
    printf("===========message sent to nxc server============\n\n");
    
    //read incoming packet from server to computer
    printf("==========receiving message from nxc server===========\n");
    char receive_buffer[BUFFER_SIZE];
    bzero(receive_buffer, BUFFER_SIZE);
    offset = 0;
    bytes = 0;
    
    /*
    do{
      bytes = read(sockfd, receive_buffer+offset, 1500);
      if(bytes < 0){
        printf("something wrong\n");
      }
      //printf("bytes is %d\n", bytes);
      offset += bytes;
      printf("offset is %d\n", offset);
      
      //EMPTY BUFFER BY SENDING
      if(offset > BUFFER_SIZE-1500){//buffer is full, must empty by sending to phone first
        printf("buffer full, cant receive any more\n");
        //send data currently in receive_buffer to phone
        length = offset;
        while(length > 0){
          int bytes_send = send(sock, receive_buffer, length, 0);
          printf("sedning %d bytes to phone\n", bytes_send);
          length=length-bytes_send;
        }
        printf("sent %s\n", receive_buffer);
        if (strncmp(receive_buffer + offset - 4, "\r\n\r\n", 4) == 0) {
          printf("reached end of message\n");
        }
        bzero(receive_buffer, BUFFER_SIZE);//empty data in receive buffer that has already been sent to phone
        offset=0;//make offset 0 to continue receiving data
      }
      else{
        if (strncmp(receive_buffer + offset - 4, "\r\n\r\n", 4) == 0) {
          printf("reached end of message\n");
          break; //if end of http
        }
        

        
      }
    } while(bytes > 0);*/
    
    do{
      bytes = read(sockfd, receive_buffer+offset, 1500);
      offset+=bytes;
      printf("offset is %d\n", offset);
      printf("buffer contents\n%s\n", receive_buffer);
      if(strncmp(receive_buffer+offset-4, "\r\n\r\n", 4) == 0) break;
      //if(strstr(receive_buffer, "\r\n\r\n")!= NULL)break;
    }while (bytes > 0);

    //bytes = read(sockfd, receive_buffer, 9000);
    //printf("message received\n");
    //printf("receive bytes : %d\n", bytes);
    //printf("%s\n", receive_buffer);
    
    if(bytes < 0){
      perror("receive error");
      exit(1);
    }
    else if (bytes == 0){
      perror("client disconneced unexpectedly");
      exit(1);
    }

    //receive_buffer[offset+1] = '\0';
    //char printbuffer[9000];
    //strncpy(printbuffer, buffer, bytes);
    //printbuffer[bytes] = '\0';
    printf("MESSAGE FROM NXC SERVER\n\n%s||||", receive_buffer);
    
    printf("========received message from nxc server==============\n\n");

    //close connection with nxc server
    printf("socket with nxc server closing\n");
    shutdown(sockconnection, SHUT_RDWR);
    close(sockconnection);
    printf("socket with nxc server closed\n\n");

    printf("=========sending message to phone===========\n");

    printf("sending %d bytes to phone\n", offset);
    bytes = send(sock, receive_buffer, offset, 0);
    if(bytes < 0){
      perror("sending error");
      exit(1);
    }
    if(bytes == 0){
      perror("fuck");
      exit(1);
    }
    if(bytes > 0){
      printf("sent successfully\n");
    }
    printf("============message sent to phone===========\n\n");

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


char *replace_str(char *str, char *orig, char *rep, int start)
{
  static char temp[BUFFER_SIZE];
  static char buffer[BUFFER_SIZE];
  char *p;

  strcpy(temp, str + start);

  if(!(p = strstr(temp, orig)))  // Is 'orig' even in 'temp'?
    return temp;

  strncpy(buffer, temp, p-temp); // Copy characters from 'temp' start to 'orig' str
  buffer[p-temp] = '\0';

  sprintf(buffer + (p - temp), "%s%s", rep, p + strlen(orig));
  sprintf(str + start, "%s", buffer);    

  return str;
}



void forward_2(int sock) {
  int offset, bytes;
  char buffer[BUFFER_SIZE];
  bzero(buffer,BUFFER_SIZE);

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

  if(strstr(buffer, "snu.nxclab.org")==NULL){//qna board -> only need do process requests regarding http://snu.nxclab.org:9000
    return; 
  }

  printf("-----------------------------------\n");
  printf("%s\n", buffer);
  
  char domain_name[100] = "snu.nxclab.org";//NEED TO MAKE THIS DYNAMIC? or is it good? idk
  
  int sockfd = 0, client_fd;
  struct sockaddr_in serv_addr;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket error");
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(9000);

  //struct sockaddr_in *sin = (struct sockaddr_in *) results->ai_addr;
  char *ip;

  // Convert IPv4 and IPv6 addresses from text to binary
  // default to nxclab's ip address for now...
  if (inet_pton(AF_INET, "147.46.132.8", &serv_addr.sin_addr) <= 0){
    printf("\nInvalid address/ Address not supported \n");
  }

  printf("%s\n", "connecting...");
  if((client_fd = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))< 0){
    perror("connect error");
  }

  //string parsing start
  char* after_slash_ptr = strstr(buffer, "snu.nxclab.org") + 19;
  char* before_http_ptr = strstr(buffer, "HTTP/1.1")-1;
  int size_of_message = offset;
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
  char remainder[BUFFER_SIZE];
  bzero(remainder, BUFFER_SIZE);
  index=0;
  for(char* i = newline_ptr; i!=buffer_end; i++){
    remainder[index] = *i;
    index+=1;
  }
  //buffer_end = "\0";
  char* messsage_to_send;
  asprintf(&messsage_to_send, "%s %s %s%s", "GET", details, "HTTP/1.1", remainder);
  //string parsing end  

  bytes = send(sockfd, messsage_to_send, strlen(messsage_to_send), 0);
  printf("message sent to server is:\n%s\n",messsage_to_send);
  printf("sent bytes : %d\n\n", bytes);

  //sleep(1);//account for various delays

  bzero(buffer,BUFFER_SIZE);
  offset = 0;
  bytes = 0;

  do {
    // bytes < 0 : unexpected error
    // bytes == 0 : client closed connection
    //receives data from socket
    bytes = read(sockfd, buffer + offset, 1500);
    offset += bytes;
    // this is end of http request
    if (strncmp(buffer + offset - 4, "\r\n\r\n", 4) == 0) break;
  } while (bytes > 0);
  //buffer[offset] = 0;

  printf("message received is:\n%s\n",buffer);
  printf("received bytes : %d\n\n", offset);
  // closing the connected socket with the nxc server
  close(sockfd);

  //forward to the local host(laptop->phone)
  printf("offset: %d\n", offset);
  /*while(offset > 0) {
    bytes = send(sock, buffer, 100, 0);
    if(bytes<0){
      bytes = 0;
    }
    printf("sent message to local host\n");
    printf("sent bytes : %d\n\n", bytes);
    offset = offset - bytes;
    printf("new offest: %d\n",offset); 
  }*/
  /*
  int itr = offset/100;
  int leftover = offset - itr*100;
  for (int i = 0;i<itr;i++){
    bytes = send(sock, buffer, 100, 0);
  }
  bytes = send(sock, buffer, leftover,0);
  */
  bytes = send(sock, buffer, offset,0);
  if(bytes == -1){
    perror("send error");
    return;
  }
  printf("sent message to local host\n");
  printf("sent bytes : %d\n\n", bytes);
  /* TODO (1): Forward received HTTP request to the web server */

  /* TODO (2): Receive responses from the web server and forward them to the browser */

  /* TODO (3): Modify HTML before forwarding response message to browser.
            Replace 20xx-xxxxx to your student id (ex. 20xx-xxxxx --> 2012-34567)
            And change the order of image displayed in browser */
}
