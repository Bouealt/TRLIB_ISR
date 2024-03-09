#ifndef TRLIB_ISRCONNECTION_H
#define TRLIB_ISRCONNECTION_H
#include"TcpConnection.h"

class ISRServer;	//类的前向声明

class ISRConnection : public TcpConnection{
public:
	static ISRConnection* createNew(ISRServer* isrser, int clientFd, std::string name = "");
	ISRConnection(ISRServer* isrser, int clientFd, std::string name);
	int sendMessage(std::string mess);
	bool isRegist() const;
	std::string getMess() const;
	void retrieveMess();
	int getFd() const;

private:
	void handleReadBytes();
	void handleWriteBytes();
	

private:
	int mFd;	//isr连接的fd
	ISRServer* mServer = NULL;
	std::string mPeerIp;
	bool mRegistFlag = false;

public:
	std::string mMess = "";
	std::string mName = "";
};

#endif // !TRLIB_ISRCONNECTION_H

