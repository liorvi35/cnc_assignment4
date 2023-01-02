#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define BUF_SIZE 1024

// command: make clean && make all && ./parta

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
    struct sockaddr_in addr;
    struct icmphdr icmp;
    char buf[BUF_SIZE];
    int len;
    struct timeval start , end; 
    memset(&start , 0 , sizeof(start));
    memset(&end , 0 , sizeof(end));

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

    while (1) {
        memset(&icmp, 0, sizeof(icmp));
        icmp.type = ICMP_ECHO;
        icmp.code = 0;
        icmp.un.echo.sequence = seq;
        icmp.un.echo.id = getpid();
        calculate_checksum(&icmp);

        gettimeofday(&start , NULL);
        
        if (sendto(sock, &icmp, sizeof(icmp), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("sendto");
            exit(EXIT_FAILURE);
        }

        len = recv(sock, buf, sizeof(buf), 0);
        if (len < 0) {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        gettimeofday(&end , NULL);
        double time = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0; //save the time in mili-seconds

        struct iphdr *ip = (struct iphdr*)buf;
        printf("  Source Address: %s\n", inet_ntoa(*(struct in_addr*)&ip->saddr));


        //  64 bytes from 8.8.8.8: icmp_seq=1 ttl=115 time=5.22 ms
        printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n", len, argv[1], icmp.un.echo.sequence , ttl, time);
        sleep(1);
        seq++;
    }
}