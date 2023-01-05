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
#include <errno.h>

#define BUF_SIZE 1024
#define IP "127.0.0.1"
#define PORT 3000
#define CONNECTIONS 50
#define OK "ok"
#define MSG_LEN 32
#define ICMP_HDRLEN 8

unsigned short checksum(void *b, int len)
{	
    unsigned short *buf = b;
	unsigned int sum=0;
	unsigned short result = 0;

	for ( sum = 0; len > 1; len -= 2 )
		sum += *buf++;
	if ( len == 1 )
		sum += *(unsigned char*)buf;

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

int main(int argc, char *argv[])
{
    int sock = 0, len = 0, seq = 0, sockfd = 0, status = 0, pid = 0;

    struct sockaddr_in addr_server, addr_ping;
    memset(&addr_server, 0, sizeof(addr_server));
    memset(&addr_ping, 0, sizeof(addr_ping));

    struct icmphdr icmp;
    memset(&icmp, 0, sizeof(icmp));

    char *args[2] = {"./watchdog", NULL}, buffer[BUFSIZ] = {'\0'};

    struct timeval start , end; 
    memset(&start , 0 , sizeof(start));
    memset(&end , 0 , sizeof(end));

    socklen_t addr_len = 0;

    double time = 0.0;

    struct iphdr *ip;
    memset(&ip , 0 , sizeof(ip));

    char recvd[IP_MAXPACKET] = {'\0'};
    size_t recvd_len = sizeof(recvd);
    
    char packet[IP_MAXPACKET] = {0} , data[MSG_LEN] = {0};
    size_t data_len = strlen(data) + 1;

    if (argc != 2) // checking that the user has specified an IP address 
    {
        printf("usage: ./partb <ip>\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < MSG_LEN - 1; i++) //the message we sent
    {
        data[i] = '0';
    }
    
    data[MSG_LEN - 1] = '\0';

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); // creating an RAW socket for ICMP communication
    if (sock <= 0) // checking if socket created
    {
        perror("socket() failed");
        close(sock);
        exit(errno);
    }

    addr_ping.sin_family = AF_INET; // setting up the struct for ICMP communicationg
    if(inet_aton(argv[1], &addr_ping.sin_addr) < 0)
    {
        perror("inet_aton() failed");
        exit(errno);
    }

    while (1)
    {
        memset(buffer, '\0', BUFSIZ);

        sockfd = socket(AF_INET, SOCK_STREAM, 0); // creating socket for TCP communcation
        if(sock <= 0) // checking if socket created
        {
            perror("socket() failed");
            close(sock);
            close(sockfd);
            exit(errno);
        }

        memset(&addr_server, '\0', sizeof(addr_server)); // setting up the struct for TCP communication
        addr_server.sin_family = AF_INET;
        addr_server.sin_port = htons(PORT);
        addr_server.sin_addr.s_addr = inet_addr(IP);

        memset(&icmp, 0, sizeof(icmp)); // setting up the struct and packet for ICMP communication
        memset(&packet , 0 , sizeof(packet));
        icmp.type = ICMP_ECHO;
        icmp.un.echo.sequence = seq;
        icmp.checksum = 0;

        memcpy((packet), &icmp, ICMP_HDRLEN);
        memcpy(packet + ICMP_HDRLEN, data, data_len);

        icmp.checksum = checksum(&packet, sizeof(packet));
        memcpy((packet), &icmp, ICMP_HDRLEN);

        gettimeofday(&start , NULL); // starting counting ping-time

        if (sendto(sock, &packet, ICMP_HDRLEN + data_len, 0, (struct sockaddr*)&addr_ping, sizeof(addr_ping)) < 0) // sending ICMP-ECHO-REQUEST
        {
            perror("sendto() failed");
            close(sock);
            exit(errno);
        }

        pid = fork(); // starting the watchdog proccess
        if (pid == 0)
        {
            execvp(args[0], args);
        }
        else if (pid > 0)
        {
            sleep(1); // delay until watchdog is setted up

            if(connect(sockfd, (struct sockaddr*)&addr_server, sizeof(addr_server)) < 0) // connecting to watchdog
            {
                perror("connect() failed");
                close(sock);
                close(sockfd);
                exit(errno);
            }

            if(send(sockfd , argv[1] , strlen(argv[1]) + 1 , 0) <= 0) // sending IP to watchdog
            {
                perror("send() failed");
                close(sock);
                close(sockfd);
                exit(errno);
            }
            
            if(recv(sockfd , buffer , BUFSIZ , 0) <= 0) // receiving an OK message from watchdog
            {
                perror("recv() failed");
                close(sock);
                close(sockfd);
                exit(errno);
            }
            if(strcmp(buffer, OK) != 0) // checking that OK received
            {
                printf("error occurred!");
                close(sock);
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            memset(buffer, '\0', BUFSIZ);

        addr_len = sizeof(addr_ping); // receiving ICMP-ECHO-REPLEY
        bzero(recvd, IP_MAXPACKET);
        len = recvfrom(sock, recvd, recvd_len, 0, (struct sockaddr *)&addr_ping, &addr_len);
        if (len == -1)
        {
            perror("recvfrom() failed");
            close(sock);
            exit(errno);
        }

            gettimeofday(&end , NULL); // ending counting ping-time


            if(send(sockfd, OK, strlen(OK) + 1 , 0) < 0) // sending OK message
            {
                perror("send() failed");
                close(sock);
                close(sockfd);
                exit(errno);
            }

            wait(&status); 
            if(status == -1)
            {
                printf("proccess status failed\n");
                close(sock);
                close(sockfd);
                exit(EXIT_FAILURE);
            }

            time = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0; // saving the time in mili-seconds
            
            time -= 1000; // removing one second because of sleep(1)

            ip = (struct iphdr*)recvd;

            printf("%d bytes has been recv from %s to " , len, inet_ntoa(*(struct in_addr*)&ip->saddr));
            printf("%s: icmp_seq=%d ttl=%d time=%.2f ms\n", inet_ntoa(*(struct in_addr*)&ip->daddr) , icmp.un.echo.sequence , ip->ttl, time);

            seq++;

            sleep(1);
        }
        else
        {
            perror("fork() failed");
            close(sock);
            close(sockfd);
            exit(errno);
        }
    }

    close(sock);
    close(sockfd);
    exit(EXIT_SUCCESS);
}
