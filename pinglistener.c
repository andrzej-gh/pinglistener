#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

#define BUF_LEN 16384

int main(int argc, char *argv[])
{
   int                sockFd;
   struct sockaddr_in sockAddr;
   socklen_t          sockAddrLen = sizeof(sockAddr);
   char               msgBuf[BUF_LEN];
   int                msgLen;
   char               ip[INET_ADDRSTRLEN];
   time_t             curTime;
   int                i;
   int                bindInt = 0;
   char               interface[5];
   int                infLoop = 1;
   int                timeLeft = 0;
   int                iterLeft = 0;
   struct ifreq       ifr;

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

   timeLeft += time(NULL);
   do {
      msgLen = recvfrom(sockFd, msgBuf, BUF_LEN, 0,
                        (struct sockaddr *) &sockAddr, &sockAddrLen);
      if (msgLen < 0) {
         perror("recvfrom");
         exit(1);
      }
      time(&curTime);
      printf("%.19s PING: Recieved %d Bytes ", ctime(&curTime), msgLen);

      inet_ntop(AF_INET, &(sockAddr.sin_addr), ip, INET_ADDRSTRLEN);
      printf("from %s\n", ip);
      iterLeft--;
   } while (iterLeft > 0 || timeLeft > curTime || infLoop);
}
