#ifndef TRLIB_SOCKETSOPS_H
#define TRLIB_SOCKETSOPS_H
#include<string>

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include"Log.h"
#define RX_SIZE 2000

#endif
//防止冲突
namespace sockets {	//命名空间sockets
	int createTcpSock();	//创建tcp套接字
	bool bind(int sockfd, std::string ip, uint16_t port);	//绑定
	bool listen(int sockfd, int backlog);	//监听
	int accept(int sockfd);	//接受
	void setNonBlockAndCloseOnExec(int sockfd);	//设置非阻塞和关闭执行
	void ignoreSigPipeOnSocket(int sockfd);	//忽略SIGPIPE信号
	void setReuseAddr(int sockfd, int on);	//设置地址重用
	void close(int sockfd);	//关闭
	int write(int sockfd, const void* buf, int size);	//写
	
}