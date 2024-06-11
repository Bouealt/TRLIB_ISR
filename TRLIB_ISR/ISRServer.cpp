#include"ISRServer.h"
#include<iostream>
#include<unistd.h>
#include<string>
#include"../Base/SocketsOps.h"
#include"../Driver/usbctl.h"
#include<fstream>
#include<fcntl.h>
#include<string.h>



ISRServer* ISRServer::createNew(EventScheduler* scheduler, ThreadPool* threadPool, InetAddress& wifiAddr, InetAddress& lanAddr, std::vector<InetAddress> serverAddrs) {
	return new ISRServer(scheduler, threadPool, wifiAddr, lanAddr, serverAddrs);
}

ISRServer::ISRServer(EventScheduler* scheduler, ThreadPool* threadPool, InetAddress& wifiAddr, InetAddress& lanAddr, std::vector<InetAddress> serverAddrs) :
	mDevice(Device::createNew(wifiAddr,lanAddr)),
	mTRServer(TRServer::createNew(serverAddrs)),
	mScheduler(scheduler),
	mThreadPool(threadPool)
{
	mCloseTriggerEvent = TriggerEvent::createNew(this, -1);//套接字设置为-1表示回调函数不需要该套接字
	mCloseTriggerEvent->setTriggerCallback(closeConnectCallback);	//处理断开连接的地址

	mReConnectTask = Task::createNew();
	mReConnectTask->setTaskCallback(reConnectTaskCallback, this);	//重新连接

	serverConnectInit(mTRServer);
	deviceConnectInit(mDevice);
	remove(mOffLinePath.c_str());	//清除离线数据文件
	std::cout << "init success" << std::endl;
	regist();
}
   
ISRServer::~ISRServer() {
	if (mWifiAcceptIOEvent) {
		mScheduler->removeIOEvent(mWifiAcceptIOEvent);
		delete mWifiAcceptIOEvent;
	}
	if (mLanAcceptIOEvent) {
		mScheduler->removeIOEvent(mLanAcceptIOEvent);
		delete mLanAcceptIOEvent;
	}
	if(mBlueToothAcceptIOEvent)
	{
		mScheduler->removeIOEvent(mBlueToothAcceptIOEvent);
		delete mBlueToothAcceptIOEvent;
	
	}
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
	delete mCloseTriggerEvent;
	delete mDevice;
}

void ISRServer::start() {
	if (-1 != mDevice->mWifiFd)
	{
		sockets::listen(mDevice->mWifiFd,20);
		mScheduler->addIOEvent(mWifiAcceptIOEvent);
	}
	if (-1 != mDevice->mLanFd)
	{
		sockets::listen(mDevice->mLanFd, 20);
		mScheduler->addIOEvent(mLanAcceptIOEvent);
	}
}
void ISRServer::bluetoothReadCallback(void* arg, int fd) {
	ISRServer* ser = (ISRServer*)arg;
	ser->handleBluetoothRead(fd);
}
void ISRServer::handleBluetoothRead(int fd) {
    char buffer[1024];
    int bytesRead = usbctl::readPort(fd, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';  // 确保字符串以 '\0' 结尾
        std::cout << "Received data from Bluetooth fd " << fd << ": " << buffer << std::endl;

        // 处理蓝牙数据
        ISRServer::handleBlueToothData(buffer, bytesRead);
    } else {
        if (bytesRead == 0) {
            std::cout << "Bluetooth connection closed by peer on fd " << fd << std::endl;
        } else {
            perror("read");
        }
        // 处理连接关闭或错误的情况
        close(fd);
    }
}

void ISRServer::handleBlueToothData(const char* data, int length) {
    // 处理蓝牙数据
    std::cout << "Handling Bluetooth data: " << std::string(data, length) << std::endl;
    // 添加蓝牙数据处理逻辑
}


void ISRServer::readCallback(void* arg, int fd) {
	ISRServer* ser = (ISRServer*)arg;
	ser->handleRead(fd);
}

void ISRServer::handleRead(int fd) {
	int clientFd = sockets::accept(fd);
	if (clientFd < 0) {
		std::cout << "accept error" << std::endl;
		return;
	}
	std::cout << "new client, fd = " << clientFd << std::endl;
	ISRConnection* conn = ISRConnection::createNew(this, clientFd);
	printf(" handle read con");
	conn->setDisConnectCallback(ISRServer::disConnectCallback, this);
	mConnectMap.insert(std::make_pair(clientFd, conn));
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
			if (conn->mSAPMac != "") {
				//断开连接的是sap
				std::cout << conn->mSAPMac << " disconnect" << std::endl;
				mSAPMacMap.erase(conn->mSAPMac);
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

void ISRServer::deviceConnectInit(Device* device) {
	if (0 == device->deviceNum) {
		std::cout << "The number of available devices : 0" << std::endl;
		//exit(-1);
	}
	if (-1 != device->mWifiFd)
	{
		mWifiAcceptIOEvent = IOEvent::createNew(device->mWifiFd, this);
		mWifiAcceptIOEvent->setReadCallback(readCallback);
		mWifiAcceptIOEvent->enableReadHandling();
	}
	if (-1 != device->mLanFd)
	{
		mLanAcceptIOEvent = IOEvent::createNew(device->mLanFd, this);
		mLanAcceptIOEvent->setReadCallback(readCallback);  
		mLanAcceptIOEvent->enableReadHandling();
	}
	if (-1 != device->mLoraFd) {
		std::cout << "lora" << std::endl;//还没测试过能不能用
		ISRConnection* conn = ISRConnection::createNew(this, device->mLoraFd);
		printf(" lora set connectcall");
		conn->setDisConnectCallback(ISRServer::disConnectCallback, this);
		mConnectMap.insert(std::make_pair(device->mLoraFd, conn));
	}
	if (-1 != device->mBlueToothFd) {
		std::cout << "bluetooth" << std::endl;
		//还没测试过
		mBlueToothAcceptIOEvent = IOEvent::createNew(device->mBlueToothFd, this);
        mBlueToothAcceptIOEvent->setReadCallback(bluetoothReadCallback);
        mBlueToothAcceptIOEvent->enableReadHandling();
	}
}

void ISRServer::serverConnectInit(TRServer* server) {
	if (0 == server->mServerNum) {
		std::cout << "The number of available servers : 0" << std::endl;
		for (auto it : server->mDisconnectServerAddrs) {
			std::cout << it.getName() << " disconnect" << std::endl;
		}
		reConnect();
	}

	for (auto it : server->mServerMap) {	//服务器连接map同步
		if (mServerMap.find(it.first) != mServerMap.end())continue;
		printf(" serverConnectInit");
		ISRConnection* conn = ISRConnection::createNew(this, it.first, mTRServer->mServerMap[it.first].getName());
		conn->setDisConnectCallback(ISRServer::disConnectCallback, this);
		mConnectMap.insert(std::make_pair(it.first, conn));
		mServerMap.insert(std::make_pair(it.first, conn));
	}
}

void ISRServer::regist() {
	std::string ISRRegisterMess = "$04500" + mDevice->mNetId + "00001F" + mDevice->mMacId + mDevice->mGps + mDevice->memR + mDevice->cpuR + mDevice->mIp + "@";
	TriggerEvent* registTriggerEvent = TriggerEvent::createNew(this, -1, ISRRegisterMess);
	registTriggerEvent->setSendCallback(sendCallback);
	mScheduler->addTriggerEvent(registTriggerEvent);
	
	TimerEvent* TimerEvent22 = TimerEvent::createNew(this, -1, "send 22 mess");
	TimerEvent22->setTimeoutCallback(timeOutCallback22);
	TimerEvent22->start();
	mScheduler->addTimerEventRunEvery(TimerEvent22, 3 * 60 * 1000);//发送22包的定时间隔为3分钟

	TimerEvent* TimerEventppp = TimerEvent::createNew(this, -1, "restart ppp");
	TimerEventppp->setTimeoutCallback(timeOutCallbackppp);
	TimerEventppp->start();
	mScheduler->addTimerEventRunEvery(TimerEventppp, 15 * 60 * 1000);//重新ppp的定时间隔为15分钟

	//注册之后，添加一个定时事件，时间到，判断mDevice中mUpdateTimeFlag是否为1，不为1则提示使用的本地时间。
}

void ISRServer::handle01Mess(void* arg, std::string sapMac) {
	if (mSAPMacMap.find(sapMac) != mSAPMacMap.end()) {
		std::cout << sapMac << " registered, send 20 mess" << std::endl;
		handle20Mess(arg, sapMac);
	}
	else {
		mSAPMacMap.insert(std::make_pair(sapMac, (ISRConnection*)arg));
		std::cout << sapMac << " register, send 17 mess" << std::endl;
		handle17Mess(arg, sapMac);
	}
}

void ISRServer::handle02Mess(void* arg, std::string sapMac) {
	std::cout << "SAP register confirm" << std::endl;
	handle05Mess(arg, sapMac);
	handle20Mess(arg, sapMac);

	ISRConnection* conn = (ISRConnection*)arg;
	TimerEvent* TimerEvent03 = TimerEvent::createNew(this, conn->getFd(), "send 03 mess");
	TimerEvent03->setTimeoutCallback(timeOutCallback03);
	TimerEvent03->start();
	mScheduler->addTimerEventRunEvery(TimerEvent03, 8 * 1000);//发送03包的定时间隔为8秒
}

void ISRServer::handle03Mess(int fd) {
	ISRConnection* conn = mConnectMap[fd];
	std::string mess03 = "$03100" + mDevice->mNetId + "00001A" + "01" + mDevice->mNetId + conn->mSAPMac + conn->mConnectWay + "@";
	TriggerEvent* handle03TriggerEvent = TriggerEvent::createNew(this, fd, mess03);//fd不为-1则表示发送给指定套接字，为-1表示发送给所有服务器
	handle03TriggerEvent->setSendCallback(sendCallback);
	mScheduler->addTriggerEvent(handle03TriggerEvent);
}

void ISRServer::handle05Mess(void* arg, std::string sapMac) {
	ISRConnection* conn = (ISRConnection*)arg;
	std::string mess05 = "$05100" + mDevice->mNetId + "00" + "0036" + mDevice->mMacId + sapMac + conn->mSAPGps + conn->mSAPCpu + conn->mSAPMem + conn->mConnectWay + conn->mSAPPort + "@";
	TriggerEvent* handle05TriggerEvent = TriggerEvent::createNew(this, -1, mess05);
	handle05TriggerEvent->setSendCallback(sendCallback);
	mScheduler->addTriggerEvent(handle05TriggerEvent);
}

void ISRServer::handle06Mess(void* arg, std::string mess) {
	if (send06Flag > 4) {
		//每收到4次06包给服务器发送一次
		TriggerEvent* handle06TriggerEvent = TriggerEvent::createNew(this, -1, mess);
		handle06TriggerEvent->setSendCallback(sendCallback);
		mScheduler->addTriggerEvent(handle06TriggerEvent);
		send06Flag = 0;
	}
	send06Flag++;
}

void ISRServer::handle10Mess(void* arg, std::string mess) {
	if (isSetTime)return;
	std::cout << "update local time" << std::endl;
	std::string serverDate = mess.substr(13, 19);
	std::string setDate = "date -s \"" + serverDate + "\"";
	system(setDate.c_str());
	isSetTime = true;
	system("echo 1 > /sys/class/leds/green/brightness");	//继电器开启
}

void ISRServer::handle17Mess(void* arg, std::string sapMac) {
	ISRConnection* conn = (ISRConnection*)arg;
	std::string mess17 = "$11700" + mDevice->mNetId + "000026" + sapMac + mDevice->mMacId + "01" + mDevice->mNetId + "@";
	TriggerEvent* handle17TriggerEvent = TriggerEvent::createNew(this, conn->getFd(), mess17);
	handle17TriggerEvent->setSendCallback(sendCallback);
	mScheduler->addTriggerEvent(handle17TriggerEvent);
}

void ISRServer::handle20Mess(void* arg, std::string sapMac) {
	ISRConnection* conn = (ISRConnection*)arg;
	std::string time = usbctl::getTime();
	std::string mess20 = "$20100" + mDevice->mNetId + "00" + "0021" + sapMac + time + "227@";
	TriggerEvent* handle20TriggerEvent = TriggerEvent::createNew(this, conn->getFd(), mess20);
	handle20TriggerEvent->setSendCallback(sendCallback);
	mScheduler->addTriggerEvent(handle20TriggerEvent);
}

void ISRServer::handle21Mess(void* arg, std::string mess) {
	mess.replace(1, 2, "06");
	std::cout << "21 -> 06 :" << mess << std::endl;
	handle06Mess(arg, mess);
}

void ISRServer::handle22Mess() {
	std::string mess22 = "$22" + mDevice->mMacId + "@";
	TriggerEvent* handle22TriggerEvent = TriggerEvent::createNew(this, -1, mess22);
	handle22TriggerEvent->setSendCallback(sendCallback);
	mScheduler->addTriggerEvent(handle22TriggerEvent);
}

void ISRServer::sendCallback(void* arg, int fd, std::string mess) {
	std::cout <<"will send mess : "<<  mess << std::endl;
	ISRServer* isrSer = (ISRServer*)arg;
	if (fd != -1) {
		//给指定套接字发送消息
		isrSer->sendMess(fd, mess);
	}
	else {
		//发送给所有连接的服务器
		isrSer->sendtoAllServer(mess);
	}
}

void ISRServer::reConnect() {
	if (!mReconnectServerTimerEvent) {
		mReconnectServerTimerEvent = TimerEvent::createNew(this, -1, "reconnect server");
		mReconnectServerTimerEvent->setTimeoutCallback(reConnectTimeoutCallback);
	}
	if (mReconnectServerTimerEvent->isStop()) {
		std::cout << "start reconnect server" << std::endl;
		mReconnectServerTimerEvent->start();
		mScheduler->addTimerEventRunEvery(mReconnectServerTimerEvent, 60 * 1000); // 60 * 1000
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
	if (!mTRServer->mDisconnectServerAddrs.empty()) {
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

void ISRServer::timeOutCallback03(void* arg, int fd) {
	ISRServer* isrser = (ISRServer*)arg;
	isrser->handle03Mess(fd);
}

void ISRServer::timeOutCallback22(void* arg, int fd) {
	ISRServer* isrser = (ISRServer*)arg;
	isrser->handle22Mess();
}

void ISRServer::timeOutCallbackppp(void* arg, int fd) {
	ISRServer* isrser = (ISRServer*)arg;
	isrser->mTRServer->pppInit();
}


void ISRServer::reConnectTimeoutCallback(void* arg, int fd) {
	ISRServer* isrser = (ISRServer*)arg;
	isrser->mThreadPool->addTask(isrser->mReConnectTask, "reconnectTask");
}


void ISRServer::reConnectTaskCallback(void* arg) {//进来了就会触发发送离线数据，得改
	ISRServer* isrser = (ISRServer*)arg;
	if (isrser->mTRServer->reConnect()) {
		std::cout << "connect all server success" << std::endl;
		isrser->mReconnectServerTimerEvent->stop();
	}
	else {
		std::cout << "connect all server faild" << std::endl;
	}
	
	isrser->serverConnectInit(isrser->mTRServer);
	if (isrser->mServerMap.size() != 0) {
		isrser->handleOffLineMess();
	}
}

void ISRServer::handleOffLineMess() {
	if (!mSendOffLineMessTimerEvent) {
		mSendOffLineMessTimerEvent = TimerEvent::createNew(this, -1, "send offline mess");
		mSendOffLineMessTimerEvent->setTimeoutCallback(sendOffLineMessCallback);
	}
	if (access(mOffLinePath.c_str(), F_OK) && mSendOffLineMessTimerEvent->isStop()) {//有离线文件且发送离线文件的定时事件未开启
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
		isrser->mSendOffLineMessTimerEvent->stop();
	}
}

void ISRServer::readOfflineMessTaskCallback(void* arg) {
	ISRServer* isrser = (ISRServer*)arg;
	std::cout << "Read OffLine Mess" << std::endl;
	std::ifstream in;
	in.open(isrser->mOffLinePath);
	if (in.is_open()) {
		char mess[2000];
		while (in.getline(mess, 2000)) {
			isrser->mOffLineMessQue.push(mess);
		}
		std::cout << "offline mess size:" << isrser->mOffLineMessQue.size() << std::endl;
		in.close();
	}
	else {
		std::cout << "in open fail" << std::endl;
	}
	
	remove(isrser->mOffLinePath.c_str());
	//isrser->mSendOffLineMessTimerEvent->stop();
}
