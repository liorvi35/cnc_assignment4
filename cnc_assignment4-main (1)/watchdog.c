#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>

#define IP "127.0.0.1"
#define PORT 3000
#define CONNECTIONS 30
#define OK "ok"

int received_echo_reply = 0; // Flag to track whether we've received an ICMP-ECHO-REPLY
char buffer[BUFSIZ] = {'\0'}; // for saving ip address

void timer_callback() 
{
  if (!received_echo_reply)
  {
    printf("server <%s> cannot be reached.\n" , buffer); // printing unreach message

    kill(0 , SIGKILL); // killing all proccesses and exiting
  }
}


int main()
{ 
    int server_sock = 0, er = 1, client_sock = 0;
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t addr_size = 0;
    struct itimerval timer;
    memset(&timer, 0, sizeof(timer));
    char buff[BUFSIZ] = {0};

    server_sock = socket(AF_INET, SOCK_STREAM, 0); // creating the listener socket
    if(server_sock <= 0) // checking if socket created
    {
        perror("socket() failed");
        close(server_sock);
        exit(errno);
    }

    if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &er, sizeof(er)) < 0) // checking if ip and port are reusable
    {
        perror("setsockopt() failed");
        close(server_sock);
        exit(errno);
    }

    server_addr.sin_family = AF_INET; // setting up the struct for TCP communication
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(IP);

    if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) // binding socket with settings
    {
        perror("bind() failed");
        close(server_sock);
        exit(errno);
    }

    if(listen(server_sock, CONNECTIONS) < 0) //listen to incoming connections
    {
        perror("listen() failed");
        close(server_sock);
        exit(errno);
    } 

    addr_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size); //accept a connection
    if(client_sock <= 0) // checking if accepted
    {
        perror("accept() failed");
        close(client_sock);
        close(server_sock);
        exit(errno);
    }

    if(recv(client_sock , buffer , BUFSIZ , 0) < 0) // receiving IP
    {
        perror("recv() failed");
        close(client_sock);
        close(server_sock);
        exit(errno);
    }
    
    if(send(client_sock, OK , strlen(OK) + 1 , 0) < 0) // sending OK message
    {
        perror("send() failed");
        close(client_sock);
        close(server_sock);
        exit(errno);
    }
    
    timer.it_value.tv_sec = 10; // setting up timer for 10 seconds
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    printf("waiting for ping response...\n");

    setitimer(ITIMER_REAL, &timer, NULL); // running the timer

    signal(SIGALRM, timer_callback);

    if(recv(client_sock , buff , BUFSIZ , 0) < 0) // receiving OK message
    {
        perror("recv() failed");
        close(client_sock);
        close(server_sock);
        exit(errno);
    }
    if(strcmp(buff, OK) != 0) // checking that OK received
    {
        printf("error occurred!");
        close(client_sock);
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    
    close(client_sock);
    close(server_sock);
    exit(EXIT_SUCCESS);
}