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

#define BUF_SIZE 1024

int received_echo_reply = 0; // Flag to track whether we've received an ICMP-ECHO-REPLY

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

void timer_callback() {
  if (!received_echo_reply) {
    printf("Timed out waiting for ICMP-ECHO-REPLY\n");
    exit(0);
  }
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

REPEATE:

  int pid = fork();

  if (pid == 0) {
    // This is the child process
    // Send an ICMP-REQUEST
    
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
    printf("after send\n");
    
    char *argv[] = {"send_icmp_request", NULL};
    //execvp("send_icmp_request", argv);
  } else {
    // This is the parent process
    // Set a timer for 10 seconds
    printf("start else\n");

    struct itimerval timer;
    timer.it_value.tv_sec = 10;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);

    signal(SIGALRM, timer_callback);

    while (1) {
        // Wait for an ICMP-ECHO-REPLY or for the timer to expire
        printf("before recv\n");
        len = recv(sock, buf, sizeof(buf), 0);
        if (len < 0) {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        gettimeofday(&end , NULL);
        received_echo_reply = 1 ;

        if (received_echo_reply) {
            printf("Received ICMP-ECHO-REPLY\n");
            received_echo_reply = 0;
            break;
        }
    }

     double time = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0; //save the time in mili-seconds

    //  64 bytes from 8.8.8.8: icmp_seq=1 ttl=115 time=5.22 ms
    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n", len, argv[1], icmp.un.echo.sequence , ttl, time);
    sleep(1);
    seq++;

    int status;
    waitpid(pid, &status, 0);
    printf("Child process terminated with status %d.\n", status);
    goto REPEATE;

  }

  return 0;
}
