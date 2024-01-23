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
	mMess = mInputBuffer.mBuffer;
	mInputBuffer.retriveveAll();
	mServer->handleMess(this, mMess);
}

void ISRConnection::handleWriteBytes() {

}

int ISRConnection::sendMessage(std::string mess) {
	const char* m = mess.c_str();
	int len = strlen(m);
	std::copy(m, m + len, mOutputBuffer.mBuffer);
	mOutputBuffer.mBufferSize = len;
	return mOutputBuffer.write(mFd);
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


