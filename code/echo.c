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

	while (1) {
		int cli_addr_len = sizeof cli_addr;
		conn_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
		if (conn_fd == -1) {
			perror("==> server: accept");
			continue;
		}
		do {
			recv_num = recv(conn_fd, buf, sizeof buf, 0);
			send(conn_fd, buf, recv_num, 0);
		} while (recv_num > 0);
		close(conn_fd);
	}
	return 0;
}
