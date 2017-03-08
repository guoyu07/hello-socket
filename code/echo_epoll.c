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
#include <sys/epoll.h>

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

	int epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("==> server: epool");
		return 4;
	}
	struct epoll_event ev, events[32];
	ev.events = EPOLLIN;
	ev.data.fd = listen_fd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
		perror("==> server: epoll_ctl");
		return 5;
	}

	while (1) {
		int nfds = epoll_wait(epollfd, events, sizeof events, -1);
		if (nfds == -1) {
			perror("==> server: epoll_wait");
			return 6;
		}
		for (int i = 0; i <= nfds; i++) {
			if (events[i].data.fd == listen_fd) {
				int cli_addr_len = sizeof cli_addr;
				conn_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
				printf("conn %d\n", conn_fd);
				fcntl(conn_fd, F_SETFL, O_NONBLOCK);
				ev.events = EPOLLIN;
				ev.data.fd = conn_fd;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_fd, &ev) == -1) {
					perror("==> server: epoll_ctl conn");
					return 5;
				}
			} else {
				int fd = events[i].data.fd;
				recv_num = recv(fd, buf, sizeof buf, 0);
				printf("asdfadf %d\n", recv_num);
				if (recv_num > 0) {
					send(fd, buf, recv_num, 0);
				} else {
					epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &events[i]);
					close(i);
				}
			}
		}
	}
	return 0;
}
