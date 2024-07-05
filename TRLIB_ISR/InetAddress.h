#ifndef TRLIB_INETADDRESS_H
#define TRLIB_INETADDRESS_H
#include<string>
#include<stdint.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include"vector"
#include<memory>

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
	std::string mIp;
	uint16_t mPort;
	struct sockaddr_in mAddr;
	std::string mName;
};


#endif // !TRLIB_INETADDRESS_H
