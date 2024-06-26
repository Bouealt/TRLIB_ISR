#include"ISRConnection.h"
#include<string>
#include<stdio.h>
#include"ISRServer.h"
#include<string.h>

static void getPeerIp(int fd, std::string& ip) {
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	getpeername(fd, (struct sockaddr*)&addr, &addrlen);
	ip = inet_ntoa(addr.sin_addr);
}

ISRConnection* ISRConnection::createNew(ISRServer* isrser, int clientFd, std::string name) {
	return new ISRConnection(isrser, clientFd, name);
}

ISRConnection::ISRConnection(ISRServer* isrser, int clientFd, std::string name) :
	TcpConnection(isrser->getScheduler(), clientFd),
	mServer(isrser),
	mFd(clientFd),
	mName(name)
{
	getPeerIp(clientFd, mPeerIp);
}

void ISRConnection::handleReadBytes() {
	std::cout << "recive mess from " << mName << " : " << mInputBuffer.mBuffer << std::endl;
	std::cout << std::endl;
	mMess = mInputBuffer.mBuffer;
	mInputBuffer.retriveveAll();
	std::string messType = mMess.substr(1, 2);	//提取消息类型
	if (messType == "01") {
		if (!analysis01Mess())return;
		mServer->handle01Mess(this, mSAPMac);
	}
	if (messType == "02") {
		if (!analysis02Mess())return;
		mServer->handle02Mess(this, mSAPMac);
	}
	if (messType == "10") {
		mServer->handle10Mess(this, mMess);
	}
	else if (messType == "06") {
		mServer->handle06Mess(this, mMess);
	}
	retrieveMess();
}

void ISRConnection::handleWriteBytes() {

}

int ISRConnection::sendMessage(std::string mess) {
	const char* m = mess.c_str();	//将string转为char*
	int len = strlen(m);
	std::copy(m, m + len, mOutputBuffer.mBuffer);
	mOutputBuffer.mBufferSize = len;
	return mOutputBuffer.write(mFd);
}

int ISRConnection::analysis01Mess() {
	int posStart = mMess.find('$');	
	int posEnd = mMess.find('@');
	if (std::string::npos == posStart || std::string::npos == posEnd) return 0;//查找该包是否完整,npos表示没找到
	mSAPPort = mMess.substr(36, 3);
	mSAPGps = mMess.substr(39, 11);
	mSAPCpu = mMess.substr(50, 2);
	mSAPMem = mMess.substr(52, 2);
	mSAPMac = mMess.substr(16, 16);
	mName = mSAPMac;
	//mConnectWay = mMess.substr(3, 1);
	return 1;
}

int ISRConnection::analysis02Mess() {
	int posStart = mMess.find('$');
	int posEnd = mMess.find('@');
	if (std::string::npos == posStart || std::string::npos == posEnd) return 0;//查找该包是否完整
	//if (mSAPMac != mMess.substr(16, 16))return 0;
	mRegistFlag = true;
	return 1;
}


bool ISRConnection::isRegist() const{
	return mRegistFlag;
}

int ISRConnection::getFd() const{
	return mFd;
}

std::string ISRConnection::getMess() const{
	return mMess;
}

void ISRConnection::retrieveMess() {
	mMess = "";
}


