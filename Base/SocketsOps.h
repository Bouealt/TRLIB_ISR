#ifndef TRLIB_SOCKETSOPS_H
#define TRLIB_SOCKETSOPS_H
#include<string>

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#define RX_SIZE 2000

#endif

namespace sockets {
	int createTcpSock();
	bool bind(int sockfd, std::string ip, uint16_t port);
	bool listen(int sockfd, int backlog);
	int accept(int sockfd);
	void setNonBlockAndCloseOnExec(int sockfd);
	void ignoreSigPipeOnSocket(int sockfd);
	void setReuseAddr(int sockfd, int on);
	void close(int sockfd);
	int write(int sockfd, const void* buf, int size);
	
}