#ifndef TRLIB_TRSERVER_H
#define TRLIB_TRSERVER_H
#include"InetAddress.h"
#include"map"

class TRServer {
public:
	static TRServer* createNew(std::vector<InetAddress> serverAddrs);
	TRServer(std::vector<InetAddress> serverAddrs);
	void pppInit();
	int reConnect();
	void disConnect(int fd);

private:
	void wanInit();
	int serversConnect(std::vector<InetAddress> addrs);

public:
	int mServerNum = 0;
	std::vector<InetAddress> mServerAddrs;
	std::vector<InetAddress> mDisconnectServerAddrs;
	std::map<int, InetAddress> mServerMap;
};

#endif // !TRLIB_TRSERVER_H
