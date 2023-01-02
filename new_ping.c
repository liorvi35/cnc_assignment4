#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include<errno.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

/* defining constants */
#define BUF_SIZE 1024
#define IP "127.0.0.1" // server's ip address
#define PORT 3000 // connection port
#define CONNECTIONS 50 // number of clients that server can listen simultaneously

// run 2 programs using fork + exec
// command: make clean && make all && ./partb

void calculate_checksum(struct icmphdr *icmp)
{
    unsigned long sum = 0;
    unsigned short *ptr = (unsigned short *)icmp;
    unsigned short chksum;

    for (int i = 0; i < sizeof(struct icmphdr) / 2; i++) {
        sum += *ptr++;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    chksum = ~sum;
    icmp->checksum = chksum;
}

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in addr , addr_server;
    struct icmphdr icmp;
    char buf[BUF_SIZE];
    int len;
    struct timeval start , end; 
    memset(&start , 0 , sizeof(start));
    memset(&end , 0 , sizeof(end));
    int ipaddr[4];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s host\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if (inet_aton(argv[1], &addr.sin_addr) == 0) {
        fprintf(stderr, "Invalid address\n");
        exit(EXIT_FAILURE);
    }

    // Set the Time-To-Live (TTL) value for the IP header
    int ttl = 64;
    if (setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    int seq  = 0;

    while(1)
    {
        /////////////////////////////////////////////////////////////////
        //tcp

        /* (2) creating TCP connection */
            memset(&addr_server, '\0', sizeof(addr));

            int sockfd = socket(AF_INET, SOCK_STREAM, 0); // creating the communication socket
            if(sock <= 0) // checking if socket created
            {
                perror("socket() failed");
                close(sockfd);
                exit(errno);
            }
            printf("socket created!\n");

            addr_server.sin_family = AF_INET; //setting up socket's used protocol, port and ip
            addr_server.sin_port = htons(PORT);
            addr_server.sin_addr.s_addr = inet_addr(IP);

        ////////////////////////////////////////////////////////////////
        memset(&icmp, 0, sizeof(icmp));
        icmp.type = ICMP_ECHO;
        icmp.code = 0;
        icmp.un.echo.sequence = seq;
        icmp.un.echo.id = getpid();
        calculate_checksum(&icmp);

        gettimeofday(&start , NULL);

        if (sendto(sock, &icmp, sizeof(icmp), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("sendto");
            close(sock);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        printf("sendto 8.8.8.8\n");

        char *args[2];
        // compiled watchdog.c by makefile
        args[0] = "./watchdog";
        args[1] = NULL;
        int status;
        int pid = fork();
        if (pid == 0)
        {
            printf("in child \n");
            execvp(args[0], args);
        }
        else if (pid > 0)
        {
            sleep(1);
            if(connect(sockfd, (struct sockaddr*)&addr_server, sizeof(addr_server)) < 0) // checking if could connect to server
            {
                perror("connect() failed");
                close(sock);
                close(sockfd);
                exit(errno);
            }
            printf("connected to server!\n");

            send(sockfd , argv[1] , strlen(argv[1]) + 1 , 0);
            printf("send %s\n" , argv[1]);
            char buffer[BUFSIZ] = {0};
            recv(sockfd , buffer , BUFSIZ , 0);

            len = recv(sock, buf, sizeof(buf), 0);
            if (len < 0) {
                perror("recv");
                exit(EXIT_FAILURE);
            }

            gettimeofday(&end , NULL);


            send(sockfd , "ok" , strlen("ok") + 1 , 0);
            printf("send ok\n");

            wait(&status); // waiting for child to finish before exiting
            printf("child exit status is: %d", status);
            if(status == -1)
            {
                close(sock);
                close(sockfd);
                printf("server <%s> cannot be reached." , argv[1]);
                exit(0);
            }

            double time = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0; //save the time in mili-seconds
            time = time - 1000;
            struct iphdr *ip = (struct iphdr*)buf;
            printf("  Source Address: %s\n", inet_ntoa(*(struct in_addr*)&ip->saddr));


            //  64 bytes from 8.8.8.8: icmp_seq=1 ttl=115 time=5.22 ms
            printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n", len, argv[1], icmp.un.echo.sequence , ttl, time);
            sleep(1);
            seq++;
        }
    }

    return 0;
}
