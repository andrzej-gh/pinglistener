#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <time.h>

#define BUF_LEN 16384

int main()
{
   int                sockFd;
   struct sockaddr_in sockAddr;
   socklen_t          sockAddrLen = sizeof(sockAddr);
   char               msgBuf[BUF_LEN];
   int                msgLen;
   char               ip[INET_ADDRSTRLEN];
   time_t             curTime;

   sockFd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
   if (sockFd < 0) {
      perror("socket");
      return -1;
   }

   do {
      msgLen = recvfrom(sockFd, msgBuf, BUF_LEN, 0,
                        (struct sockaddr *) &sockAddr, &sockAddrLen);
      if (msgLen < 0) {
         perror("recvfrom");
         return -1;
      }
      time(&curTime);
      printf("%.19s PING: Recieved %d Bytes ", ctime(&curTime), msgLen);

      inet_ntop(AF_INET, &(sockAddr.sin_addr), ip, INET_ADDRSTRLEN);
      printf("from %s\n", ip);
   } while (1);

   return 0;
}
