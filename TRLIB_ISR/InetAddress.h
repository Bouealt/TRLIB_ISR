#ifndef TRLIB_INETADDRESS_H
#define TRLIB_INETADDRESS_H
#include<string>
#include<stdint.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include"vector"

class InetAddress {
public:
	InetAddress();
	InetAddress(std::string ip, uint16_t port, std::string name);

	void setAddr(std::string ip, uint16_t port);
	std::string getIp();
	uint16_t getPort();
	struct sockaddr* getAddr();
	std::string getName();

private:
	std::string mIp;	//ip地址	
	uint16_t mPort;	//端口号
	struct sockaddr_in mAddr;	//套接字地址结构体
	std::string mName;	//服务器名字
};


#endif // !TRLIB_INETADDRESS_H
