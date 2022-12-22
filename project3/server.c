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

void forward (int sock, int* connectfail);
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
  //newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  int connectfail = 0;
  while (1) {
    printf("enter while loop in main function=======================================\n");
    
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    printf("new socket connection : %d\n", newsockfd);
    //printf("create new phone laptop socket connection\n");
    if ( newsockfd == -1 ){
      perror("socket made wrong");
      exit(1);
    }
    if(newsockfd == 0){
      printf("phone computer connection weird\n");
      //close(newsockfd);
      //continue;
    }
  /*
    if(connectfail == 1){
      close(newsockfd);
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
      connectfail = 0;
      printf("SOCKET CONNECTION CLOSED AND REOPNED\n");
    }
    */
    //printf("new accept created\n");
    forward(newsockfd, &connectfail);
    close(newsockfd);

    //printf("shutdown accept\n");
    //shutdown(newsockfd, SHUT_RDWR);
    
    //printf("close server in main function============================================\n\n\n");
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

void forward(int sock, int *connectfail) {
  int offset, bytes;
  char buffer[BUFFER_SIZE];
  bzero(buffer,BUFFER_SIZE);
  printf("connection id is (enter loop) : %d\n", sock);
  offset = 0;
  bytes = 0;
  do {
    bytes = recv(sock, buffer + offset, 1500, 0);
    offset += bytes;
    
    if (strncmp(buffer + offset - 4, "\r\n\r\n", 4) == 0) {
      //printf("end found\n");
      break;
    }
  } while (bytes > 0);

  if (offset < 0) {
    printf("recv() error\n");
    *connectfail = 1;
    return;
  } else if (bytes == 0) {
    printf("RECEIVE FROM PHONE Client disconnected unexpectedly\n");
    sock = -1;
    *connectfail = 1;
    return;
  }
  
  printf("-----------------------------------\n");
  
  //for(int i=0; i<strlen(buffer); i++){
  // printf("%02x ", buffer[i]);
  //}
  printf("\n");

  if(strstr(buffer, "snu.nxclab.org")!=NULL){//if from phone to computer
    printf("%s", buffer);
    printf("offset is %d bytes\n", offset);
    //printf("phone asking computer for nxclab\n");
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
   
    //printf("value of ip addr: %.*s\n", (int)results->ai_addrlen, inet_ntoa(sin->sin_addr));

    sockfd = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
    if(sockfd == -1){
      perror("socket error");
      exit(1);
    }

    //printf("socket created\n");
    if(sockconnection = connect(sockfd, results->ai_addr, results->ai_addrlen) == -1){
      perror("connection failed");
      exit(1);
    }
    freeaddrinfo(results);
    //printf("server connected\n");

    //string manipulation, remove domain, leave only the thing after / ex) /index.html
    char* after_slash_ptr = strstr(buffer, ":9000") + 5;
    char* before_http_ptr = strstr(buffer, "HTTP/1.1")-1;
    //int size_of_message = offset;
    char details[BUFFER_SIZE];
    bzero(details, BUFFER_SIZE);
    int detail_length = before_http_ptr - after_slash_ptr;
    //printf("detail length is %d\n", detail_length);

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
    bzero(remainder, BUFFER_SIZE);
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

    char *message_to_send;
    asprintf(&message_to_send, "%s%s%s%s", "GET ", details, " HTTP/1.1", remainder);
    //printf("AHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n%.*s\n", offset-26, message_to_send);

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
    
    /*while(length > 0){
      bytes = send(sockfd, message_to_send, length, 0);
      printf("send bytes : %d\n", bytes);
      length = length - bytes;
    }*/
    bytes = send(sockfd, message_to_send, length, 0);
    printf("=============message to send to nxcserver============\n\n%s", message_to_send);
    //bytes = send(sockfd, message_to_send, length, 0);
    printf("===========message sent to nxc server============\n\n");
    
    //read incoming packet from server to computer
    printf("==========receiving message from nxc server===========\n");
    char receive_buffer[BUFFER_SIZE];
    bzero(receive_buffer, BUFFER_SIZE);
    
    bytes = 0;
    
    
    char* badreq;
    offset = 0;
    //attempted timeout on read... doesnt work
    /*
    int rx_len;
    struct timeval timeout;
    fd_set readFds;

    timeout.tv_sec = 0;
    timeout.tv_usec = 1000000;

    FD_ZERO(&readFds);
    FD_SET(sockfd, &readFds);
    select(sockfd+1, &readFds, NULL, NULL, &timeout);

    do{
      //printf("REEEEEEEEEEEEE\n");
      //configure read timeout
      
      //printf("REEEEEEEEEEEEE\n");
      select(sockfd+1, &readFds, NULL, NULL, &timeout);
      if(FD_ISSET(sockfd, &readFds)){
        bytes = read(sockfd, receive_buffer+offset, BUFFER_SIZE);
      }
      if(bytes > 0) {
        //printf("RRRRRRRRRRRRRRRRRRRE\n");
        //bytes = read(sockfd, receive_buffer+offset, BUFFER_SIZE);
        //if(bytes == 0) break;
        offset+=bytes;
        printf("\nreading %d bytes from from socket\noffset is %d\n", bytes, offset);
        if(bytes < 2000) printf("=======================================================buffer contents\n\n%s\n", receive_buffer+offset-bytes);
        else printf("too long, cant be bothered\n");
        if(strncmp(receive_buffer+offset-4, "\r\n\r\n", 4) == 0) break;
        //printf("ahahh\n");
        //if bad request comes in, just ignore
        if((badreq = strstr(receive_buffer+offset-bytes, "HTTP/1.1 400")) != NULL) {
          printf("bad request found, exiting\n");
          offset = badreq -receive_buffer;
          break;
        }
      }
      else{
        printf("message timeout sending\n");
        break;
      }
      //if(strstr(receive_buffer, "\r\n\r\n")!= NULL)break;
    }while (bytes > 0);
    */
    do{
      //printf("RRRRRRRRRRRRRRRRRRRE\n");
      bytes = read(sockfd, receive_buffer+offset, BUFFER_SIZE);
      offset+=bytes;
      printf("\nreading %d bytes from from socket\noffset is %d\n", bytes, offset);
      if(bytes < 2000) printf("=======================================================buffer contents\n\n%s\n", receive_buffer+offset-bytes);
      else printf("too long, cant be bothered\n");

      if(strncmp(receive_buffer+offset-4, "\r\n\r\n", 4) == 0) break;

      //if bad request comes in, just ignore
      if((badreq = strstr(receive_buffer+offset-bytes, "HTTP/1.1 400")) != NULL) {
        printf("bad request found, exiting\n");
        offset = badreq -receive_buffer;
        break;
      }
    
    }while (bytes > 0);
    /*
    do{
      bytes = read(sockfd, receive_buffer, BUFFER_SIZE);
      offset += bytes;
      printf("message received\n");
      printf("receive bytes : %d\n", bytes);
      if(bytes>5000){
        printf("dont bother printing\n");
      }
      else{
        printf("buffer content\n%.*s\n", offset, receive_buffer);
      }
    }while(bytes > 0);
    */
    //printf("%s\n", receive_buffer);
    
    if(bytes < 0){
      perror("receive error");
      exit(1);
    }
    else if (bytes == 0){
      perror("READING FROM NXCclient disconneced unexpectedly");
      exit(1);
    }

    //receive_buffer[offset+1] = '\0';
    //char printbuffer[9000];
    //strncpy(printbuffer, buffer, bytes);
    //printbuffer[bytes] = '\0';
    //printf("MESSAGE FROM NXC SERVER\n\n%s||||", receive_buffer);
    
    printf("========received message from nxc server==============\n\n");

    //close connection with nxc server
    printf("socket with nxc server closing\n");
    shutdown(sockconnection, SHUT_RDWR);
    close(sockconnection);
    printf("socket with nxc server closed\n\n");

    printf("=========sending message to phone===========\n");

    printf("sending %d bytes to phone\n", offset);
    
    if(offset < 10000){
      printf("the message to send\n");
      for(int i=0; i<offset; i++){
        printf("%c", receive_buffer[i]);
      }
    }
    else{
      printf("part of the message to send\n");
      for(int i=offset-2000; i<offset; i++){
        printf("%c", receive_buffer[i]);
      }
    }
    printf("\n");
    bytes = send(sock, receive_buffer, offset, 0);

    /*
    while(offset > 0){
      printf("connection id is (send) : %d\n", sock);
      bytes = send(sock, receive_buffer, offset, 0);
      offset = offset - bytes;
      printf("sent %d bytes at once\n", bytes);
      if(bytes < 0){
        perror("\nsending error");
        exit(1);
      }
      if(bytes == 0){
        perror("something error??? idk");
        exit(1);
      }
    }*/
    
    if(bytes > 0){
      printf("sent successfully\n");
    }
    printf("============message sent to phone===========\n\n");
    //printf("connection id is (after send) : %d\n", sock);
    //close(sock);
    //printf("socket connection phone - computer closed\n");

  }
  
  else{
    //close(sock);
    //printf("close phone laptop socket connection\nbecause it wasn't nxclab\n");
    printf("not nxclab, exit to main==========================================\n");
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

