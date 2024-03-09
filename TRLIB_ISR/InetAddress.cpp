#include"InetAddress.h"

InetAddress::InetAddress() {}

InetAddress::InetAddress(std::string ip, uint16_t port, std::string name) :
	mName(name)
{
	setAddr(ip, port);
}

void InetAddress::setAddr(std::string ip, uint16_t port) {
	mIp = ip;	
	mPort = port;
	mAddr.sin_family = AF_INET;	//协议簇
	mAddr.sin_addr.s_addr = inet_addr(ip.c_str());	//inet_addr将点分十进制的ip地址转换为网络字节序的32位二进制数
	mAddr.sin_port = htons(port);	//htons将主机字节序转换为网络字节序
}

std::string InetAddress::getIp() {
	return mIp;
}

uint16_t InetAddress::getPort() {
	return mPort;
}

/*许多套接字API函数（如bind，connect，accept等）需要一个sockaddr类型的指针作为参数。
我们通常使用sockaddr_in结构体来设置IP地址和端口，因为它有具体的sin_addr和sin_port字段。
然后，当我们需要将地址信息传递给套接字函数时，我们将sockaddr_in结构体强制转换为sockaddr结构体。*/

struct sockaddr* InetAddress::getAddr() {
	return (struct sockaddr*)&mAddr;	//返回sockaddr_in结构体的指针，强转为sockaddr
}

std::string InetAddress::getName() {
	return mName;
}