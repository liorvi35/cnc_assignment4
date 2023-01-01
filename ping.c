#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <errno.h>
#include <netinet/ip.h>
#include <sys/time.h> // gettimeofday()
#include <sys/types.h>

#define BUF_SIZE 1024

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
    
    char data[IP_MAXPACKET] = "This is the ping.\n";

    int datalen = strlen(data) + 1;

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

    while (1) {
        memset(&icmp, 0, sizeof(icmp));
        icmp.type = ICMP_ECHO;
        icmp.code = 0;
        icmp.un.echo.sequence = 1;
        icmp.un.echo.id = getpid();
        calculate_checksum(&icmp);


        int bytes_sent = sendto(sock, &icmp, sizeof(icmp), 0, (struct sockaddr *)&addr, sizeof(addr));
        if (bytes_sent < 0) {
            perror("sendto");
            exit(EXIT_FAILURE);
        }
        //printf("Successfuly sent one packet : ICMP HEADER : %d bytes, data length : %d , icmp header : %ld \n", bytes_sent, len, sizeof(icmp));

        len = recv(sock, buf, sizeof(buf), 0);
        if (len < 0) {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        //printf("Successfuly received one packet with %ld bytes : data length : %d , icmp header : %d , ip header : %d \n", len, datalen, ICMP_HDRLEN, IP4_HDRLEN);

        sleep(1);
    }

    close(sock);
    exit(EXIT_SUCCESS);
}