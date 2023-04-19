#include <arpa/inet.h>
#include <linux/icmp.h>
#include <linux/ip.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#define BUF_LEN 16384

int  bindInt;
char interface[5];
int  infLoop;
int  iterLeft;
int  forceQuit;

void *
listener(void * data)
{
   time_t             curTime;
   int                sockFd;
   struct sockaddr_in sockAddr;
   socklen_t          sockAddrLen = sizeof(sockAddr);
   char               msgBuf[BUF_LEN];
   int                msgLen;
   char               ip[INET_ADDRSTRLEN];
   struct ifreq       ifr;
   struct iphdr      *ipHdr;
   struct icmphdr    *icmpHdr;

   sockFd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
   if (sockFd < 0) {
      perror("socket");
      exit(1);
   }

   if (bindInt == 1) {
      printf("Binding socket to interface %s\n", interface);
      memset(&ifr, 0, sizeof(ifr));
      strncpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name));
      if (setsockopt(sockFd, SOL_SOCKET, SO_BINDTODEVICE,
                     (void *)&ifr, sizeof(ifr)) < 0) {
         perror("setsockopt");
         exit(1);
      }
   }

   printf("Starting listening...\n");
   fflush(stdout);

   do {
      msgLen = recvfrom(sockFd, msgBuf, BUF_LEN, 0,
                        (struct sockaddr *) &sockAddr, &sockAddrLen);
      if (msgLen < 0) {
         perror("recvfrom");
         exit(1);
      }

      time(&curTime);
      inet_ntop(AF_INET, &(sockAddr.sin_addr), ip, INET_ADDRSTRLEN);

      ipHdr = (struct iphdr *) msgBuf;
      icmpHdr = (struct icmphdr *) ((char *) ipHdr + (4 * ipHdr->ihl));
      if (icmpHdr->type == ICMP_ECHO) {
         printf("%.19s PING: Recieved %d Bytes from %s\n",
                ctime(&curTime), msgLen, ip);
      }

      fflush(stdout);
      iterLeft--;
   } while (iterLeft > 0 || infLoop);

   forceQuit = 1;
}

void *
quiter(void *data)
{
   char option = ' ';

   while (1) {
      scanf("%c", &option);
      if (option == 'q') {
          forceQuit = 1;
          break;
      }
   }
}

int main(int argc, char *argv[])
{
   time_t    curTime;
   int       i, timeLeft = 0;
   pthread_t quiterT;
   pthread_t listenerT;

   bindInt   = 0;
   infLoop   = 1;
   iterLeft  = 0;
   forceQuit = 0;

   for (i = 1; i < argc; i++) {
      if (i + 1 == argc || argv[i][0] != '-') {
         printf("Invalid input\n");
         exit(1);
      }

      switch (argv[i][1]) {
      case 'c':
         iterLeft = atoi(argv[++i]);
         infLoop = 0;
         break;
      case 't':
         timeLeft = atoi(argv[++i]);
         infLoop = 0;
         break;
      case 'i':
         strncpy(interface, argv[++i], 5);
         bindInt = 1;
         break;
      default:
         printf("Unknown option\n");
         exit(1);
      }
   }

   pthread_create(&quiterT, NULL, quiter, NULL);
   pthread_create(&listenerT, NULL, listener, NULL);

   timeLeft += time(NULL);
   while ((timeLeft > curTime || infLoop || iterLeft > 0) && !forceQuit) {
     sleep(1);
     time(&curTime);
   }

   printf("Bye...\n");
}
