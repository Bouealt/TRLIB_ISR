#include"ISRServer.h"
#include<iostream>
#include<unistd.h>
#include<string>
#include<string.h>
#include"../Base/Log.h"
#include"../Base/SocketsOps.h"
#include"../Driver/usbctl.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<fstream>
#include<queue>



ISRServer* ISRServer::createNew(EventScheduler* scheduler, ThreadPool* threadPool, std::vector<InetAddress> serverAddrs, InetAddress mAddr) {
	return new ISRServer(scheduler, threadPool, serverAddrs, mAddr);
}

ISRServer::ISRServer(EventScheduler* scheduler, ThreadPool* threadPool, std::vector<InetAddress> serverAddrs, InetAddress mAddr) :
	mTRServer(TRServer::createNew(serverAddrs)),
	mScheduler(scheduler),
	mThreadPool(threadPool),
	mAddr(mAddr)
{
	std::cout << mAddr.getIp() <<" "<<mAddr.getPort()<< std::endl;
	mFd = sockets::createTcpSock();//非阻塞的描述符
	sockets::setReuseAddr(mFd, 1);//设置套接字选项（重用端口）（重启时可能端口并未释放，加上后可以及时重用端口）
	if (!sockets::bind(mFd, mAddr.getIp(), mAddr.getPort())) {
		std::cout << "bind error" << std::endl; 
		//_exit(1);
	}
	mAcceptIOEvent = IOEvent::createNew(mFd, this);
	mAcceptIOEvent->setReadCallback(readCallback);//设置回调的socket可读 函数指针
	mAcceptIOEvent->enableReadHandling();//激活回调函数（只关心可读时间）

	mCloseTriggerEvent = TriggerEvent::createNew(this, -1);//套接字设置为-1表示回调函数不需要该套接字
	mCloseTriggerEvent->setTriggerCallback(closeConnectCallback);

	mReConnectTask = Task::createNew();
	mReConnectTask->setTaskCallback(reConnectTaskCallback, this);

	serverConnectInit(mTRServer);
	remove(mOffLinePath.c_str());
}

ISRServer::~ISRServer() {
	delete mAcceptIOEvent;
	delete mCloseTriggerEvent;
	if (mReconnectServerTimerEvent) {
		delete mReconnectServerTimerEvent;
	}
	if (mSendOffLineMessTimerEvent) {
		delete mSendOffLineMessTimerEvent;
	}
	if (mReConnectTask) {
		delete mReConnectTask;
	}
	if (mReadOfflineMessTask) {
		delete mReadOfflineMessTask;
	}
}

void ISRServer::start() {
	sockets::listen(mFd, 60);
	std::cout << "start" << std::endl;
	mScheduler->addIOEvent(mAcceptIOEvent);
}

void ISRServer::readCallback(void* arg, int fd) {
	std::cout << "readCallback" << std::endl;
	ISRServer* ser = (ISRServer*)arg;
	ser->handleRead(fd);
}

void ISRServer::handleRead(int fd) {
	int clientFd = sockets::accept(mFd);
	if (clientFd < 0) {
		LOGE("accept error");
		return;
	}
	LOGI("%d", clientFd);
	ISRConnection* conn = ISRConnection::createNew(this, clientFd);//有点不安全，创建客户端连接时，一定不要传名字
	conn->setDisConnectCallback(ISRServer::disConnectCallback, this);
	mConnectMap.insert(std::make_pair(clientFd, conn));
}

void ISRServer::serverConnectInit(TRServer* server) {
	if (0 != server->mDisconnectServerAddrs.size()) {
		for (auto it : server->mDisconnectServerAddrs) {
			std::cout << it.getName() << " disconnect" << std::endl;
		}
		reConnect();
	}
	for (auto it : server->mServerMap) {
		if (mServerMap.find(it.first) != mServerMap.end())continue;
		ISRConnection* conn = ISRConnection::createNew(this, it.first, mTRServer->mServerMap[it.first].getName());//传服务器名才会认为创建服务器连接
		conn->setDisConnectCallback(ISRServer::disConnectCallback, this);
		mConnectMap.insert(std::make_pair(it.first, conn));
		mServerMap.insert(std::make_pair(it.first, conn));
	}
}

void ISRServer::disConnectCallback(void* arg, int fd) {
	ISRServer* isrser = (ISRServer*)arg;
	isrser->handleDisConnect(fd);
}

void ISRServer::handleDisConnect(int fd) {
	std::lock_guard <std::mutex> lck(mMtx);
	mDisConnectList.push_back(fd);
	mScheduler->addTriggerEvent(mCloseTriggerEvent);
}

void ISRServer::closeConnectCallback(void* arg, int fd) {
	ISRServer* isrser = (ISRServer*)arg;
	isrser->handleCloseConnect();
}

void ISRServer::handleCloseConnect() {
	std::lock_guard<std::mutex> lck(mMtx);
	for (auto it : mDisConnectList) {
		int fd = it;
		if (mConnectMap.find(fd) != mConnectMap.end()) {
			ISRConnection* conn = mConnectMap[fd];
			if (conn->mName == "") {
				//断开连接的是客户端
				std::cout << "client " << " disconnect" << std::endl;
			}
			else {
				//断开的是服务器
				mTRServer->disConnect(fd);
				mServerMap.erase(fd);
			}
			delete conn;
			mConnectMap.erase(fd);
		}
	}
	mDisConnectList.clear();
}

void ISRServer::handleMess(void* arg, std::string mess) {
	std::cout << "handleMess" << std::endl;
	TriggerEvent* handleMessTriggerEvent = TriggerEvent::createNew(this, -1, mess);
	handleMessTriggerEvent->setSendCallback(sendCallback);
	mScheduler->addTriggerEvent(handleMessTriggerEvent);
}

void ISRServer::sendCallback(void* arg, int fd, std::string mess) {
	std::cout <<"will send mess : "<<  mess << std::endl;
	ISRServer* isrSer = (ISRServer*)arg;
	if (fd == -1) {
		//发送给所有连接的服务器
		isrSer->sendtoAllServer(mess);
	}
	else {
		//给指定套接字发送消息
		isrSer->sendMess(fd, mess);
	}
}

void ISRServer::reConnect() {
	if (!mReconnectServerTimerEvent) {
		mReconnectServerTimerEvent = TimerEvent::createNew(this, -1);
		mReconnectServerTimerEvent->setTimeoutCallback(reConnectTimeoutCallback);
	}
	if (mReconnectServerTimerEvent->isStop()) {
		std::cout << "start reconnect server" << std::endl;
		mReconnectServerTimerEvent->start();
		mScheduler->addTimerEventRunEvery(mReconnectServerTimerEvent, 1 * 10 * 1000);
	}
}

void ISRServer::sendtoAllServer(std::string mess) {
	if (!mServerMap.empty())
	{
		for (auto it : mServerMap) {
			sendMess(it.first, mess);
		}
	}
	else {
		//保存到本地
		std::cout << "save offline mess, mess = " << mess << std::endl;
		int fd = open(mOffLinePath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0777);
		mess += "\r\n";
		int ret = write(fd, mess.c_str(), strlen(mess.c_str()));
		if (ret > 0) {
			std::cout << "save offline success" << std::endl;
		}
		else {
			std::cout << "save offline error" << std::endl;
		}
		close(fd);
	}
	if(!mTRServer->mDisconnectServerAddrs.empty()){
		//重连服务器
		reConnect();
	}
}

void ISRServer::sendMess(int fd, std::string mess) {
	int ret = mConnectMap[fd]->sendMessage(mess);
	std::string messType = mess.substr(1, 2);
	if (ret <= 0) {
		std::cout << "send " << messType << " mess to " << mConnectMap[fd]->mName << " error" << std::endl;
	}
	else {
		std::cout << "send " << messType << " mess to " << mConnectMap[fd]->mName << " success" << std::endl;
	}
}

void ISRServer::reConnectTimeoutCallback(void* arg, int fd) {
	ISRServer* isrser = (ISRServer*)arg;
	isrser->mThreadPool->addTask(isrser->mReConnectTask, "reconnectTask");
}

void ISRServer::reConnectTaskCallback(void* arg) {
	ISRServer* isrser = (ISRServer*)arg;
	if (isrser->mTRServer->reConnect()) {
		std::cout << "connect all server success" << std::endl;
		isrser->mReconnectServerTimerEvent->stop();
	}
	else {
		std::cout << "connect all server faild" << std::endl;
	}
	isrser->serverConnectInit(isrser->mTRServer);
	isrser->handleOffLineMess();
	
}

void ISRServer::handleOffLineMess() {
	if (!mSendOffLineMessTimerEvent) {
		mSendOffLineMessTimerEvent = TimerEvent::createNew(this, -1);
		mSendOffLineMessTimerEvent->setTimeoutCallback(sendOffLineMessCallback);
	}
	if (access(mOffLinePath.c_str(), F_OK) && mSendOffLineMessTimerEvent->isStop()) {
		return;
	}
	if (!mReadOfflineMessTask) {
		mReadOfflineMessTask = Task::createNew();
		mReadOfflineMessTask->setTaskCallback(readOfflineMessTaskCallback, this);
	}
	mThreadPool->addTask(mReadOfflineMessTask, "ReadOfflineTask");

	mSendOffLineMessTimerEvent->start();
	mScheduler->addTimerEventRunEvery(mSendOffLineMessTimerEvent, 4 * 1000);
}

void ISRServer::sendOffLineMessCallback(void* arg, int fd) {
	ISRServer* isrser = (ISRServer*)arg;
	if (!isrser->mOffLineMessQue.empty()) {
		isrser->sendtoAllServer(isrser->mOffLineMessQue.front());
		isrser->mOffLineMessQue.pop();
	}
	else {
		std::cout << "stop" << std::endl;
		isrser->mSendOffLineMessTimerEvent->stop();
	}
}

void ISRServer::readOfflineMessTaskCallback(void* arg) {
	ISRServer* isrser = (ISRServer*)arg;
	std::cout << "Read OffLine Mess" << std::endl;
	std::ifstream in;
	in.open(isrser->mOffLinePath);
	char mess[200];
	while (in.getline(mess, 200)) {
		isrser->mOffLineMessQue.push(mess);
	}
	std::cout << "size:" << isrser->mOffLineMessQue.size() << std::endl;
	remove(isrser->mOffLinePath.c_str());
	//isrser->mSendOffLineMessTimerEvent->stop();
}