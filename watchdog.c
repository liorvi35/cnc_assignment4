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


void timer_callback() {
  if (!received_echo_reply) {
    printf("Timed out waiting for ICMP-ECHO-REPLY\n");
    exit(EXIT_FAILURE);
  }
}

int main()
{
    printf("hello partb\n");

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

    struct sockaddr_in server_addr;
    
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
    struct sockaddr_in client_addr;
    socklen_t addr_size = 0;
    addr_size = sizeof(client_addr);
    int client_sock = 0;
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size); //accept a connection
    if(client_sock <= 0) // checking if accepted
    {
        perror("bind() failed");
        close(client_sock);
        close(server_sock);
        exit(errno);
    }
    printf("sender connected!\n");

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
        recv(client_sock , buffer , BUFSIZ , 0);
        close(client_sock);
        return EXIT_SUCCESS;
    }
    return 0;
}
// 1. ping 
// 2. open watchdog(start clock)
// 3.recv ping 
// 4. send watchdog OK
// 5. check if in time 
// 5.a. if yes - send ok and repeate 
// 5.b. if no - send time out 