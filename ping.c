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

void calculate_checksum(struct icmphdr *icmp)
{
    unsigned long sum = 0;
    unsigned short *ptr = (unsigned short *)icmp, chksum = 0;

    for (int i = 0; i < sizeof(struct icmphdr) / 2; i++)
    {
        sum += *ptr++;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    chksum = ~sum;
    
    icmp->checksum = chksum;
}

int main(int argc, char *argv[])
{
    int sock = 0, len = 0, seq = 0;

    struct sockaddr_in addr_ping;
    memset(&addr_ping, 0, sizeof(addr_ping));

    struct icmphdr icmp;
    memset(&icmp, 0, sizeof(icmp));

    char buffer[BUFSIZ] = {'\0'};

    struct timeval start , end; 
    memset(&start , 0 , sizeof(start));
    memset(&end , 0 , sizeof(end));

    socklen_t addr_len = 0;

    double time = 0.0;

    struct iphdr *ip;
    memset(&ip , 0 , sizeof(ip));

    if (argc != 2) // checking that the user has specified an IP address 
    {
        printf("usage: ./partb <ip>\n");
        exit(EXIT_FAILURE);
    }

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
        memset(&icmp, 0, sizeof(icmp)); // setting up the struct for ICMP communication
        icmp.type = ICMP_ECHO;
        icmp.code = 0;
        icmp.un.echo.sequence = seq;
        icmp.un.echo.id = getpid();
        calculate_checksum(&icmp);

        gettimeofday(&start , NULL); // starting counting ping-time

        if (sendto(sock, &icmp, sizeof(icmp), 0, (struct sockaddr*)&addr_ping, sizeof(addr_ping)) < 0) // sending ICMP-ECHO-REQUEST
        {
            perror("sendto() failed");
            close(sock);
            exit(errno);
        }

        addr_len = sizeof(addr_ping); // receiving ICMP-ECHO-REPLEY
        len = recvfrom(sock, buffer, BUFSIZ, 0, (struct sockaddr*)&addr_ping, &addr_len);
        if (len < 0) 
        {
            perror("recvfrom() failed");
            close(sock);
            exit(errno);
        }

        gettimeofday(&end , NULL); // ending counting ping-time

        time = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0; // saving the time in mili-seconds
        
        ip = (struct iphdr*)buffer;

        printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n", 
            len, inet_ntoa(*(struct in_addr*)&ip->saddr), icmp.un.echo.sequence , ip->ttl, time);
        
        seq++;

        sleep(1);
    }

    close(sock);
    exit(EXIT_SUCCESS);
}