#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "base64.c"

#define PORT 25000 // You can change port number here

int respond (int sock);

char usrname[] = "username";
char password[] = "password";
#include <stdint.h>
#include <stdlib.h>
//Problem 1 of project 1:simple webserver with authentification
//Both Problem 1 and 2 was tested on WSL enviroments, Linux, and M1 mac
//But If you still have problems on running codes please mail us
//Most importantly please comment your code

//If you are using mac 
//You can install homebrew here :https://brew.sh
//And open terminal and type 
//sudo brew install gcc
//sudo brew install make
//Type make command to build server
//And server binary will be created
//Use ifconfig command to figure out your ip(usually start with 192. or 172.)
//run server with ./server and open browser and type 192.x.x.x:25000



//If you are using Linux or WSL
//You just need to run "make"(If you are using WSL you may need to install gcc and make with apt)
//And server binary will be created
//Use ifconfig command to figure out your ip(usually start with 192. or 172.)
//run server with ./server and open browser and type 192.x.x.x:25000


//It will be better if you run virtual machine or other device to run server
//But you can also test server with opening terminal and run it on local IP 

int send_exception(int sock){

}


int main( int argc, char *argv[] ) {
  int sockfd, newsockfd, portno = PORT;
  socklen_t addrlen;
  struct sockaddr_in serv_addr, cli_addr;
  addrlen = sizeof(cli_addr);

  int buffer_size = 9000;
  char buf[buffer_size];
  int ret;

  char auth[] = "admin:admin";
  printf("encoding start \n");// We have implemented base64 encoding you just need to use this function
  char *token = base64_encode(auth, strlen(auth));//you can change your userid
  printf("encoding end \n");  

  //browser will repond with base64 encoded "userid:password" string 
  //You should parse authentification information from http 401 responese and compare it


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

  bzero((char*) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
    perror("bind failed");
    exit(1);
  }
  /* TODO : Now bind the host address using bind() call. 10% of score*/
    //it was mostly same as tutorial

  if(listen(sockfd, 10)<0){
    perror("listed failed");
    exit(1);
  }
  /* TODO : listen on socket you created  10% of score*/


  printf("Server is running on port %d\n", portno);
    
  //it was mostly same as tutorial
  //in the while loop every time request comes we respond with respond function if valid
  //add some random comment
  //TODO: authentication loop 40 % of score
  int bytes, offset;
  bzero(buf, buffer_size);

  int authenticated = 0;
  
  while(1){//authentication loop
    
    if((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &addrlen))<0){
      perror("accept error");
      exit(1);
    }
    //TODO: accept connection
    //TODO: send 401 message(more information about 401 message : https://developer.mozilla.org/en-US/docs/Web/HTTP/Authentication) and authentificate user
    //close connection

    
    offset = 0;
    bytes = 0;

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
    
    buf[offset] = 0;
    printf("%s\n", buf);

    char auth_message[] = "HTTP/1.1 401 Unauthorized \r\nWWW-Authenticate: Basic realm = \"Access to the staging site\"";
    int length = strlen(auth_message);

    while(length > 0){//send 401 message
      printf("send bytes : %d\n", bytes);
      bytes = send(newsockfd, auth_message, length, 0);
      length = length - bytes;
    }
    printf("close\n");
    shutdown(newsockfd, SHUT_RDWR);
    close(newsockfd);

    if(authenticated == 1){
      printf("authentication successful\n");
      //send OK
      
      break;
    }

    if(authenticated == 0){//while not authenticated, send auth message to client
      //check if authentication successful
      printf("token is : %s\n", token);
      printf("auth not succecssful\n\n");
      if(strstr(buf, token) != NULL){//token found inside the buffer
        authenticated = 1;
      }
    }
  }
  //Respond loop
  while (1) {
    printf("\n\nin respond loop\n");
    if((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &addrlen))<0){
      perror("accept error");
      exit(1);
    }
    respond(newsockfd);
    printf("close\n");
    shutdown(newsockfd, SHUT_RDWR);
    close(newsockfd);
  }
  return 0;
}
//TODO: complete respond function 40% of score
int respond(int sock) {
  int offset, bytes;
  char receive_buffer[9000];
  bzero(receive_buffer,9000);

  offset = 0;
  bytes = 0;
  do {
    // bytes < 0 : unexpected error
    // bytes == 0 : client closed connection
    bytes = recv(sock, receive_buffer + offset, 1500, 0);
    offset += bytes;
    // this is end of http request
    if (strncmp(receive_buffer + offset - 4, "\r\n\r\n", 4) == 0) break;
  } while (bytes > 0);

  if (bytes < 0) {
    printf("recv() error\n");
    exit(1);
  } else if (bytes == 0) {
    printf("Client disconnected unexpectedly\n");
    exit(1);
  }

  receive_buffer[offset] = 0;
  printf("%s\n", receive_buffer);
  char *path;

  //parsing string for path
  if(strstr(receive_buffer, "GET") != NULL){//if message has GET in it
    printf("get found\n");
    char* first_space = strchr(receive_buffer, ' ')+1; //first instance of ' '
    char* second_space = strchr(first_space, ' ');//second instance of ' '
    printf("index is %ld\n", second_space - first_space);
    
    path = malloc(second_space - first_space);

    /* Copy the strings into our memory */
    strncpy(path, first_space,  second_space - first_space);
    //printf("path is |%s|\n", path);

    /* Null terminators (because strncpy does not provide them) */
    path[second_space-first_space] = 0;
    //printf("path is |%s|\n", path);
  }

  int is_png;
  int is_css;
  int is_js;
  if(strstr(path, "png")!=NULL){
    is_png = 1;
  }
  else if(strstr(path, "css")!=NULL){
    is_css = 1;
  }
  else if(strstr(path, "js")!=NULL){
    is_js = 1;
  }
  int length;
  char* final_path;//final path which includes . in front
  void *send_buffer;//buffer that holds data from server
  char png_buffer[100000];//buffer for if file is png
  FILE *sendFile;
  if(strcmp(path, "/") == 0) {
    printf("index.html opened\n");
    sendFile = fopen("index.html", "r");
  }
  else{
    asprintf(&final_path, ".%s", path);
    printf("%s opened\n", final_path);
    sendFile = fopen(final_path, "r");
    /*
    if(is_png == 0){
      printf("non png file \n");
      sendFile = fopen(final_path, "r");
    }
    else{//if file is png use rb
      printf("png file\n");
      sendFile = fopen(final_path, "rb");
    }*/
  }

  free(path);
  printf("file opened\n");
  if(sendFile == NULL){
    //if no file found, give 400 error
    perror("no file found\n");
    char error_message[] = "HTTP/1.1 400 Bad request";
    int error_message_length = strlen(error_message);
    while(error_message_length > 0){
      bytes = send(sock, error_message, error_message_length, 0);
      printf("send error message bytes : %d\n", bytes);
      error_message_length = error_message_length - bytes;
    }
    return 0;
  }

  fseek(sendFile, 0, SEEK_END);//set file pointer to end to find length
  length = ftell(sendFile);
  printf("length is %d\n", length);
  fseek(sendFile, 0, SEEK_SET);//set file pointer to the beginning

  /*
  if(is_png == 0 ){
    send_buffer = malloc(length);
  }
  if(is_png == 1){
    fread(png_buffer, 1, length, sendFile);
    //send_buffer = png_buffer;
    printf("contents of png_buffer \n%s\n", png_buffer);
  }*/
  send_buffer = malloc(length);
  int send_buffer_length = length;
  printf("send buffer length is %d\n", send_buffer_length);
  if(send_buffer > 0){
    fread(send_buffer, 1, length, sendFile);
    printf("reading from buffer\n");
  }
  /*
  if(is_png == 1){
    for(int i=0; i<length; i++){
      printf("%02X", send_buffer[i]);
    }
  }*/
  printf("\n");
  fclose(sendFile);

  char *prefix;
  char *suffix = "\r\n\r\n";
  if(is_png){//if PNG file
    is_png = 0;
    prefix = "HTTP/1.1 200 OK\r\nContent-Type: image/png;\r\n\r\n";
    int prefix_length = strlen(prefix);
    int suffix_length = strlen(suffix);
    while(prefix_length > 0){
      bytes = send(sock, prefix, prefix_length, 0);
      printf("send prefix bytes : %d\n", bytes);
      prefix_length = prefix_length - bytes;
    }
    while(length > 0){
      bytes = send(sock, send_buffer, length, 0);
      printf("send buffer bytes : %d\n", bytes);
      length = length - bytes;
    }
    while(suffix_length > 0){
      bytes = send(sock, suffix, suffix_length, 0);
      printf("send suffix bytes : %d\n", bytes);
      suffix_length = suffix_length - bytes;
    }
    return 0;
  }
  else if(is_css){
    prefix = "HTTP/1.1 200 OK\r\nContent-Type: text/css;\r\n\r\n";
  }
  else if(is_js){
    prefix = "HTTP/1.1 200 OK\r\nContent-Type: text/html;\r\n\r\n";
  }
  else{
    prefix = "HTTP/1.1 200 OK\r\nContent-Type: text/html;\r\n\r\n";
  }
  
  char* message;
  void* png_message;
  //printf("prefix is %s\n", prefix);
  //is_png = 0;
  //fwrite(send_buffer, length, 1, stdin);
  //send_buffer[length]=0;
  if((asprintf(&message,"%s%s%s", prefix, (char*)send_buffer, suffix) < 0)){
    //printf("contents of entire message with header and prefix\n%s\n", message);
    printf("concat fail\n");
  }
  length = strlen(message);
  /*
  else{//cannot use asprintf for non text file, do manually
    //printf("prefix is |%s|\n", prefix);
    printf("prefix length %ld, send buffer length %d\n", strlen(prefix), send_buffer_length);
    int first_part_length = strlen(prefix) + send_buffer_length+1;
    printf("first part length %d\n", first_part_length);
    void *first_part = malloc(first_part_length);//allocate space for prefix and file + 1 for terminating string
    
    printf("check1\n");
    memcpy(first_part, prefix, strlen(prefix));//first part holds prefix
    fwrite(first_part, 1, strlen(prefix), stdout);//print what is currently in first_part
    printf("check2\n");
    memcpy(first_part, send_buffer, send_buffer_length);//concat image onto prefix
    printf("check3\n");
    
    //fwrite(first_part, 1, first_part_length, stdout);//print what is in firstpart after adding image

    int message_length = first_part_length+strlen(suffix);
    printf("check4\n");
    png_message = malloc(message_length);//allocate memory for full message
    printf("check5\n");
    strncpy(png_message, first_part, first_part_length);
    printf("check6\n");
    strncat(png_message, suffix, strlen(suffix));
    printf("check7\n");
    length = message_length;
    printf("full message length %d\n", length);
    free(first_part);
    printf("check8\n");
  }
  */
  free(send_buffer);
  //printf("check9\n");
  
  bytes= 0;
  
  //length = strlen(message);
  printf("length of message %d\n", length);
  while(length > 0){
    bytes = send(sock, message, length, 0);
    printf("send bytes : %d\n", bytes);
    length = length - bytes;
  }
  is_png = 0;
  is_css = 0;
  is_js = 0;
  return 0;
}

