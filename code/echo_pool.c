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
#include <pool.h>

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

	struct poolfd fds[1024];
	int fdmax, nready;
	fds[0].fd = listen_fd;
	fds[0].events = POLLRDNORM; // monitor read
	fdmax = 0;
	for (int i = 1; i < 1024; i++) {
		fds[i].fd = -1;
	}

	while (1) {
		if (nready = pool(fds, fdmax + 1, NULL, INFTIM) == -1) {
			perror("==> server: pool");
			return 4;
		}
		if (fds[0].revents & POLLRDNORM) {
			int cli_addr_len = sizeof cli_addr;
			conn_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
			fcntl(conn_fd, F_SETFL, O_NONBLOCK);
			if (conn_fd > fdmax) {
				fdmax = conn_fd;
			}
			for (int i = 0; i < 1024; i++) {
				if (fds[i].fd < 0) {
					fds[i].fd = conn_fd;
					fds[i].events = POLLRDNORM;
					break;
				}
			}
			if (i == 1024) {
				perror("==> server: max conn")
					return 5;
			}
			if (--nready <= 0) continue;
		}
		for (int i = 1; i <= fdmax; i++) {
			int fd = fds[i].fd;
			if (fd < 0) continue;
			if (fds[i].revents & POLLRDNORM) {
				recv_num = recv(fd, buf, sizeof buf, 0);
				if (recv_num > 0) {
					send(fd, buf, recv_num, 0);
				} else {
					fds[i].fd = -1;
					close(i);
				}
			}
		}
	}
	return 0;
}
