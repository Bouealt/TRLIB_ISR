#include"TRServer.h"
#include"../Driver/deCell4g.h"
#include"../Base/Log.h"
#include<iostream>
#include<unistd.h>

TRServer* TRServer::createNew(std::vector<InetAddress> serverAddrs) {
	return new TRServer(serverAddrs);
}

TRServer::TRServer(std::vector<InetAddress> serverAddrs) :
	mServerAddrs(serverAddrs)
{
	mServerNum = serversConnect(mServerAddrs);
}

int TRServer::serversConnect(std::vector<InetAddress> addrs) {
	mDisconnectServerAddrs.clear();
	int serverNum = 0;
	for (auto it : addrs) {
		int fd = deCell::ipPortCheck(it.getIp().c_str(), it.getPort());
		std::string serverName = it.getName();
		if (-1 == fd) {
			std::cout << "connect " << serverName << " error" << std::endl;
			mDisconnectServerAddrs.push_back(it);
		}
		else {
			std::cout << "connect " << serverName << " success" << std::endl;
			mServerMap.insert(std::make_pair(fd, it));
			serverNum++;
		}
	}
	return serverNum;
}

int TRServer::reConnect() {
	mServerNum += serversConnect(mDisconnectServerAddrs);
	return mServerAddrs.size() == mServerNum;
}

void TRServer::disConnect(int fd) {
	std::cout << mServerMap[fd].getName() << " disconnect" << std::endl;
	mServerNum--;
	mDisconnectServerAddrs.push_back(mServerMap[fd]);
	mServerMap.erase(fd);	//删除已断开连接的服务器
}