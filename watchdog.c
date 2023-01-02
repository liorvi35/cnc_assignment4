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

int received_echo_reply = 0; // Flag to track whether we've received an ICMP-ECHO-REPLY
char buff[BUFSIZ] = {0};

void timer_callback() {
  if (!received_echo_reply) {
    printf("server <%s> cannot be reached.\n" , buff);
    kill(0 , SIGKILL);
  }
}


int main()
{


////////////////////////////////////    
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, '\0', sizeof(server_addr));
    memset(&client_addr, '\0', sizeof(client_addr));
    socklen_t addr_size = 0;

    int server_sock = socket(AF_INET, SOCK_STREAM, 0); // creating the listener socket
    if(server_sock <= 0) // checking if socket created
    {
        perror("socket() failed");
        close(server_sock);
        exit(errno);
    }
    printf("socket created!\n");

    int er = 1;
    if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &er, sizeof(er)) < 0) // // checking if ip and port are reusable
    {
        perror("setsockopt() failed");
        close(server_sock);
        exit(errno);
    }

    server_addr.sin_family = AF_INET; //setting up socket's used protocol, port and ip
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(IP);

    if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) // binding socket with settings
    {
        perror("bind() failed");
        close(server_sock);
        exit(errno);
    }
    printf("socket bound!\n");

    if(listen(server_sock, CONNECTIONS) < 0) //listen to incoming connections
    {
        perror("listen() failed");
        close(server_sock);
        exit(errno);
    } 
    printf("waiting for connection...\n");

    /* (2) getting connection from the sender */
    addr_size = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size); //accept a connection
    printf("the accept is: %d\n" , client_sock);
    if(client_sock <= 0) // checking if accepted
    {
        perror("accept() failed");
        close(client_sock);
        close(server_sock);
        exit(errno);
    }
    printf("sender connected!\n");

    recv(client_sock , buff , BUFSIZ , 0);

///////////////////////////////////

    printf("start timer\n");
    struct itimerval timer;
    timer.it_value.tv_sec = 10;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);

    signal(SIGALRM, timer_callback);

    while (1)
    {
        char buffer[BUFSIZ] = {0};
        printf("before recv ping\n");
        recv(client_sock , buffer , BUFSIZ , 0);
        printf("after recv ping\n");
        close(client_sock);
        close(server_sock);
        return 0;
    }

    return 0;
}
