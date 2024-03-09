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
	mCloseTriggerEvent->setTriggerCallback(closeConnectCallback);	//设置断开回调

	mReConnectTask = Task::createNew();
	mReConnectTask->setTaskCallback(reConnectTaskCallback, this);	//设置重连回调

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
	conn->setDisConnectCallback(ISRServer::disConnectCallback, this);	//设置断开连接回调函数
	mConnectMap.insert(std::make_pair(clientFd, conn));
}

void ISRServer::serverConnectInit(TRServer* server) {
	if (0 != server->mDisconnectServerAddrs.size()) {	//有断开的连接
		for (auto it : server->mDisconnectServerAddrs) {
			std::cout << it.getName() << " disconnect" << std::endl;
		}
		reConnect();
	}
	for (auto it : server->mServerMap) {	//查找TRserver中的服务器连接是否存在
		if (mServerMap.find(it.first) != mServerMap.end())continue;	//已经连接的服务器，跳过，it.first是fd
		ISRConnection* conn = ISRConnection::createNew(this, it.first, mTRServer->mServerMap[it.first].getName());//传服务器名才会认为创建服务器连接
		conn->setDisConnectCallback(ISRServer::disConnectCallback, this);	//设置断开连接回调函数
		mConnectMap.insert(std::make_pair(it.first, conn));
		mServerMap.insert(std::make_pair(it.first, conn));
	}
}

void ISRServer::disConnectCallback(void* arg, int fd) {
	ISRServer* isrser = (ISRServer*)arg;
	isrser->handleDisConnect(fd);
}

void ISRServer::handleDisConnect(int fd) {
	std::lock_guard <std::mutex> lck(mMtx);	//加锁，防止多线程操作公共资源
	mDisConnectList.push_back(fd);	//将断开的fd加入到list中
	mScheduler->addTriggerEvent(mCloseTriggerEvent);	//添加触发事件，将执行closeConnectCallback
}

void ISRServer::closeConnectCallback(void* arg, int fd) {
	ISRServer* isrser = (ISRServer*)arg;
	isrser->handleCloseConnect();	//处理断开连接
}

void ISRServer::handleCloseConnect() {
	std::lock_guard<std::mutex> lck(mMtx);
	for (auto it : mDisConnectList) {
		int fd = it;
		if (mConnectMap.find(fd) != mConnectMap.end()) {	//存在
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
	handleMessTriggerEvent->setSendCallback(sendCallback);	//设置发送回调
	mScheduler->addTriggerEvent(handleMessTriggerEvent);	//添加触发事件
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
	if (!mReconnectServerTimerEvent) {	//服务器重连定时器为空
		mReconnectServerTimerEvent = TimerEvent::createNew(this, -1);
		mReconnectServerTimerEvent->setTimeoutCallback(reConnectTimeoutCallback);	//设置超时回调
	}
	if (mReconnectServerTimerEvent->isStop()) {	//定时器停止，重启
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
		int ret = write(fd, mess.c_str(), strlen(mess.c_str()));	//写入文件
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
	std::string messType = mess.substr(1, 2);	//获取消息类型
	if (ret <= 0) {
		std::cout << "send " << messType << " mess to " << mConnectMap[fd]->mName << " error" << std::endl;
	}
	else {
		std::cout << "send " << messType << " mess to " << mConnectMap[fd]->mName << " success" << std::endl;
	}
}

void ISRServer::reConnectTimeoutCallback(void* arg, int fd) {	//重连超时回调
	ISRServer* isrser = (ISRServer*)arg;
	isrser->mThreadPool->addTask(isrser->mReConnectTask, "reconnectTask");	//将重连任务添加到线程池中，mReConnectTask为Task对象
}

void ISRServer::reConnectTaskCallback(void* arg) {	//重连任务回调
	ISRServer* isrser = (ISRServer*)arg;
	if (isrser->mTRServer->reConnect()) {
		std::cout << "connect all server success" << std::endl;
		isrser->mReconnectServerTimerEvent->stop();	//停止重连定时器
	}
	else {
		std::cout << "connect all server faild" << std::endl;
	}
	isrser->serverConnectInit(isrser->mTRServer);	//初始化服务器连接
	isrser->handleOffLineMess();	//处理离线消息
}

void ISRServer::handleOffLineMess() {
	if (!mSendOffLineMessTimerEvent) {	//发送离线消息定时器为空
		mSendOffLineMessTimerEvent = TimerEvent::createNew(this, -1);
		mSendOffLineMessTimerEvent->setTimeoutCallback(sendOffLineMessCallback);	//设置超时回调
	}
	if (access(mOffLinePath.c_str(), F_OK) && mSendOffLineMessTimerEvent->isStop()) {	//离线文件不存在且定时器停止
		return;
	}
	if (!mReadOfflineMessTask) {	//读离线消息任务为空
		mReadOfflineMessTask = Task::createNew();
		mReadOfflineMessTask->setTaskCallback(readOfflineMessTaskCallback, this);	//设置读离线消息回调
	}
	mThreadPool->addTask(mReadOfflineMessTask, "ReadOfflineTask");

	mSendOffLineMessTimerEvent->start();	//启动发送离线消息定时器
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