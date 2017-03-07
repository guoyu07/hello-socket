#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
  int sockfd, numbytes, rv;
  char ip[INET6_ADDRSTRLEN];
  char buf[1024];
  struct addrinfo hints, *servinfo, *p;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo("bilibili.com", "http", &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("==> client: socket");
      continue;
    }
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      perror("==> client: connect");
      continue;
    }
    break;
  }

  if (p == NULL) {
    perror("==> client: failed to connect");
    return 2;
  }
  inet_ntop(p->ai_family, p->ai_addr, ip, sizeof ip);
  printf("==> client: connected to %.*s\n", sizeof ip, ip);

  char req[] = "GET / HTTP/1.0\r\nHost: bilibili.com\r\n\r\n";
  send(sockfd, req, sizeof req, 0);
  if ((numbytes = recv(sockfd, buf, 1024, 0)) == -1) {
    perror("==> client: recv");
    return 3;
  }
  printf("==> client: received\n%.*s", numbytes, buf);

  close(sockfd);

  return 0;
}
