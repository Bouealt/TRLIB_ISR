#include"TcpConnection.h"
#include"../Base/SocketsOps.h"

TcpConnection::TcpConnection(EventScheduler* scheduler, int clientFd):
	mScheduler(scheduler),
	mFd(clientFd)
{
	mIOEvent = IOEvent::createNew(mFd, this);
	mIOEvent->setReadCallback(readCallback);
	mIOEvent->setWriteCallback(writeCallback);
	mIOEvent->setErrorCallback(errorCallback);
	mIOEvent->enableReadHandling();
	mIOEvent->enableWriteHandling();

	mScheduler->addIOEvent(mIOEvent);
}

TcpConnection::~TcpConnection() {
	mScheduler->removeIOEvent(mIOEvent);
	delete mIOEvent;
	sockets::close(mFd);
}

void TcpConnection::setDisConnectCallback(DisConnectCallback cb, void* arg) {
	mDisConnectCallback = cb;
	mArg = arg;
}

void TcpConnection::enableReadHandling() {
	if (mIOEvent->isReadHandling())return;
	mIOEvent->enableReadHandling();
	mScheduler->updateIOEvent(mIOEvent);
}

void TcpConnection::enableWriteHandling() {
	if (mIOEvent->isWriteHandling())return;
	mIOEvent->enableWriteHandling();
	mScheduler->updateIOEvent(mIOEvent);
}

void TcpConnection::enableErrorHandling() {
	if (mIOEvent->isErrorHandling())return;
	mIOEvent->enableErrorHandling();
	mScheduler->updateIOEvent(mIOEvent);
}

void TcpConnection::disableReadHandling() {
	if (!mIOEvent->isReadHandling())return;
	mIOEvent->disableReadeHandling();
	mScheduler->updateIOEvent(mIOEvent);
}

void TcpConnection::disableWriteHandling() {
	if (!mIOEvent->isWriteHandling())return;
	mIOEvent->disableWriteHandling();
	mScheduler->updateIOEvent(mIOEvent);
}

void TcpConnection::disableErrorHandling() {
	if (!mIOEvent->isErrorHandling())return;
	mIOEvent->disableErrorHandling();
	mScheduler->updateIOEvent(mIOEvent);
}

void TcpConnection::handleRead() {
	int ret = mInputBuffer.read(mFd);
	if (ret <= 0) {
		LOGE("read error, disconnect, fd = %d,ret = %d", mFd, ret);
		handleDisConnect();
		return;
	}
	handleReadBytes();
}

void TcpConnection::handleWrite() {
	handleWriteBytes();
}

void TcpConnection::handleError() {
	LOGE("error fd = %d", mFd);
}

void TcpConnection::handleReadBytes() {

}

void TcpConnection::handleWriteBytes() {
	LOGE("write fd = %d", mFd);
}

void TcpConnection::handleDisConnect() {
	if (mDisConnectCallback) {
		mDisConnectCallback(mArg, mFd);
	}
}

void TcpConnection::readCallback(void* arg, int fd) {
	TcpConnection* conn = (TcpConnection*)arg;
	conn->handleRead();
}

void TcpConnection::writeCallback(void* arg, int fd) {
	TcpConnection* conn = (TcpConnection*)arg;
	conn->handleWrite();
}

void TcpConnection::errorCallback(void* arg, int fd) {
	TcpConnection* conn = (TcpConnection*)arg;
	conn->handleError();
}
