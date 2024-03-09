#include"TcpConnection.h"
#include"../Base/SocketsOps.h"

TcpConnection::TcpConnection(EventScheduler* scheduler, int clientFd):
	mScheduler(scheduler),
	mFd(clientFd)
{
	mIOEvent = IOEvent::createNew(mFd, this);	//创建IO事件
	mIOEvent->setReadCallback(readCallback);	//设置事件回调函数
	mIOEvent->setWriteCallback(writeCallback);
	mIOEvent->setErrorCallback(errorCallback);
	mIOEvent->enableReadHandling();
	mIOEvent->enableWriteHandling();

	mScheduler->addIOEvent(mIOEvent);	//添加IO事件
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
	if (mIOEvent->isReadHandling())return;	//判断是否开启
	mIOEvent->enableReadHandling();	//设置IO事件的读处理
	mScheduler->updateIOEvent(mIOEvent);	//更新IO事件
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

void TcpConnection::handleRead() {	//回调函数实例
	int ret = mInputBuffer.read(mFd);	//从文件描述符中读取数据到输入缓冲区中
	if (ret <= 0) {
		LOGE("read error, disconnect, fd = %d,ret = %d", mFd, ret);
		handleDisConnect();	//断开连接
		return;
	}
	handleReadBytes();
}

void TcpConnection::handleWrite() {
	handleWriteBytes();	//写数据
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
