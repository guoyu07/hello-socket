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
#include <fcntl.h>

int main(int argc, char *argv[])
{
  int listen_fd, conn_fd, recv_num;
  struct sockaddr_in serv_addr, cli_addr;
  char buf[1024];

  listen_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (listen_fd == -1) {
    perror("==> server: socket");
    return 1;
  }

  memset(&serv_addr, 0, sizeof serv_addr);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(1234);

  if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof serv_addr) == -1) {
    perror("==> server: bind");
    return 2;
  }

  if (listen(listen_fd, 1) == -1) {
    perror("==> server: listen");
    return 3;
  }

  fd_set fds, fds2;
  int fdmax;
  fcntl(listen_fd, F_SETFL, O_NONBLOCK);

  FD_ZERO(&fds);
  FD_SET(listen_fd, &fds);
  fdmax = listen_fd;

  while (1) {
    fds2 = fds;
    if (select(fdmax+1, &fds2, NULL, NULL, NULL) == -1) {
      perror("==> server: select");
      return 4;
    }
    for (int i = 0; i <= fdmax; i++) {
      if (!FD_ISSET(i, &fds2)) {
        continue;
      }
      if (i == listen_fd) {
        int cli_addr_len = sizeof cli_addr;
        conn_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
        fcntl(conn_fd, F_SETFL, O_NONBLOCK);
        FD_SET(conn_fd, &fds);
        if (conn_fd > fdmax) {
          fdmax = conn_fd;
        }
      } else {
        recv_num = recv(i, buf, sizeof buf, 0);
        if (recv_num > 0) {
          send(i, buf, recv_num, 0);
        } else {
          FD_CLR(i, &fds);
          close(i);
        }
      }
    }
  }
  return 0;
}
