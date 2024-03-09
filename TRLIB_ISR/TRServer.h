#ifndef TRLIB_TRSERVER_H
#define TRLIB_TRSERVER_H
#include"InetAddress.h"
#include"map"

class TRServer {
public:
	static TRServer* createNew(std::vector<InetAddress> serverAddrs);
	TRServer(std::vector<InetAddress> serverAddrs);
	int reConnect();
	void disConnect(int fd);

private:
	int serversConnect(std::vector<InetAddress> addrs);

public:
	int mServerNum = 0;	//连接的数量
	std::vector<InetAddress> mServerAddrs;	//所有服务器地址
	std::vector<InetAddress> mDisconnectServerAddrs;	//已断开连接的服务器地址
	std::map<int, InetAddress> mServerMap;	//服务器fd和地址的映射
};

#endif // !TRLIB_TRSERVER_H
