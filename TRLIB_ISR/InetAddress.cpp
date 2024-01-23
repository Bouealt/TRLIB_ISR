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
	mAddr.sin_family = AF_INET;
	mAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	mAddr.sin_port = htons(port);
}

std::string InetAddress::getIp() {
	return mIp;
}

uint16_t InetAddress::getPort() {
	return mPort;
}

struct sockaddr* InetAddress::getAddr() {
	return (struct sockaddr*)&mAddr;
}

std::string InetAddress::getName() {
	return mName;
}