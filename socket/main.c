#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/tcp.h>

int main()
{
	int sfd, fd;
    struct addrinfo *ai;
    struct addrinfo *next;
    struct addrinfo hints = { .ai_flags = AI_PASSIVE,
                              .ai_family = AF_UNSPEC };
	struct linger ling = {0, 0};
	int error;
	int success = 0;
	char *interface = NULL ;
	int port = 22224 ;
	char port_buf[NI_MAXSERV];
	struct sockaddr addr;
	socklen_t addrlen;
	int flags = 1 ;
	int fs;

	hints.ai_socktype = SOCK_STREAM ;
	snprintf(port_buf, sizeof(port_buf), "%d", port);
	error= getaddrinfo(interface, port_buf, &hints, &ai);
	if (error != 0) {
        if (error != EAI_SYSTEM)
          fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(error));
        else
          perror("getaddrinfo()");
        return 1;
    }
	for (next= ai; next; next= next->ai_next) {
		if ((sfd = socket(next->ai_family, next->ai_socktype, next->ai_protocol)) == -1){
			perror("server_socket") ;
			exit(1) ;
		}
		if ((fs = fcntl(sfd, F_GETFL, 0)) < 0 ||
				fcntl(sfd, F_SETFL, fs | O_NONBLOCK) < 0) {
			perror("setting O_NONBLOCK");
			close(sfd);
			return -1;
		}
		setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
		error = setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags));
		if (error != 0)
			perror("setsockopt");

		error = setsockopt(sfd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));
		if (error != 0)
			perror("setsockopt");

		error = setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));
		if (error != 0)
			perror("setsockopt");

		if (bind(sfd, next->ai_addr, next->ai_addrlen) == -1){
			if (errno != EADDRINUSE) {
				perror("bind()");
				close(sfd);
				freeaddrinfo(ai);
				return 1;
			}
			close(sfd);
			continue;
		}
		listen(sfd,10);
		while (1){
			addrlen = sizeof(addr);
			fd = accept(sfd,&addr,&addrlen) ;
			char * wstr = "this is test" ;
			write(fd,wstr,strlen(wstr)) ;
			close(fd) ;
		}
	}
	freeaddrinfo(ai);
}
