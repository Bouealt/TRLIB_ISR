#include"SocketsOps.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>

int sockets::createTcpSock() {
	int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);	//ipv4, tcp
	return sockfd;
}

bool sockets::bind(int sockfd, std::string ip, uint16_t port) {
	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip.c_str());	//inet_addr将点分十进制的ip地址转换为网络字节序的32位二进制数
	addr.sin_port = htons(port);	//htons将主机字节序转换为网络字节序
	if (::bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		return false;
	}
	return true;
}

bool sockets::listen(int sockfd, int backlog) {
	if (::listen(sockfd, backlog) < 0) {	//::作用域解析，这里是全局作用域
		LOGE("listen error");
		return false;
	}
	return true;
}

int sockets::accept(int sockfd) {	//接受新请求
	struct sockaddr_in addr = { 0 };	//用于存储新信息
	socklen_t addrlen = sizeof(struct sockaddr_in);

	int connfd = ::accept(sockfd, (struct sockaddr*)&addr, &addrlen);	//返回值是新连接的文件描述符
	setNonBlockAndCloseOnExec(connfd);
	ignoreSigPipeOnSocket(connfd);
	return connfd;
}


void sockets::setNonBlockAndCloseOnExec(int sockfd)
{
	// non-block
	int flags = ::fcntl(sockfd, F_GETFL, 0);	//获取状态标志
	flags |= O_NONBLOCK;	//将非阻塞添加到标志位中
	int ret = ::fcntl(sockfd, F_SETFL, flags);	//设置状态标志

	// close-on-exec
	flags = ::fcntl(sockfd, F_GETFD, 0);//fork子进程时，限制子进程对于设置了close-on-exec文件的权限
	flags |= FD_CLOEXEC;	//在执行exec系列函数时，关闭文件描述符
	ret = ::fcntl(sockfd, F_SETFD, flags);
}

void sockets::ignoreSigPipeOnSocket(int socketfd)
{	//当试图写入一个已经关闭的socket时，会产生SIGPIPE信号，这个信号的默认处理是终止进程，这里忽略SIGPIPE信号
	int option = 1;
	setsockopt(socketfd, SOL_SOCKET, MSG_NOSIGNAL, &option, sizeof(option));
}

void sockets::setReuseAddr(int sockfd, int on)
{
	//设置套接字选项：SOL_SOCKET：指定要设置选项的协议层，此处为套接字选项。SO_REUSEADDR：要设置的选项名称，表示允许重用本地地址。

	int optval = on ? 1 : 0;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));//这个套接字选项通常在服务器程序中使用，以便在服务器关闭后能够快速重新启动并绑定到之前使用的地址和端口上，而无需等待一段时间。
}

void sockets::close(int sockfd) {
	int ret = ::close(sockfd);
}

int write(int sockfd, const void* buf, int size) {
	return ::write(sockfd, buf, size);
}