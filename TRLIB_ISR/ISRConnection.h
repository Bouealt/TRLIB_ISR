#ifndef TRLIB_ISRCONNECTION_H
#define TRLIB_ISRCONNECTION_H
#include "TcpConnection.h"

class ISRServer;

class ISRConnection : public TcpConnection
{
public:
	static ISRConnection *createNew(ISRServer *isrser, int clientFd, std::string name = "");
	ISRConnection(ISRServer *isrser, int clientFd, std::string name);
	int sendMessage(std::string mess);
	bool isRegist() const;
	std::string getMess() const;
	void retrieveMess();
	int getFd() const;

private:
	void handleReadBytes();
	void handleWriteBytes();
	int analysis01Mess();
	int analysis02Mess();

private:
	int mFd;
	ISRServer *mServer = NULL;
	std::string mPeerIp;
	bool mRegistFlag = false;

public:
	std::string mSAPPort = "";
	std::string mSAPGps = "";
	std::string mSAPCpu = "";
	std::string mSAPMem = "";
	std::string mMess = "";
	std::string mSAPMac = "";
	std::string mConnectWay = "0001";
	std::string mName = "";

public:
};

#endif // !TRLIB_ISRCONNECTION_H
