#ifndef TRLIB_ISRSERVER_H
#define TRLIB_ISRSERVER_H
#include<mutex>
#include"../Base/Event.h"
#include"../Base/EventScheduler.h"
#include"ISRConnection.h"
#include"InetAddress.h"
#include"Device.h"
#include"TRServer.h"
#include"../Base/ThreadPool.h"
#include<queue>
class ISRServer {
public:
	static ISRServer* createNew(EventScheduler* scheduler, ThreadPool* threadPool, std::vector<InetAddress> serverAddrs, InetAddress mAddr);
	ISRServer(EventScheduler* scheduler, ThreadPool* threadPool, std::vector<InetAddress> serverAddrs, InetAddress mAddr);
	~ISRServer();
	void start();
	EventScheduler* getScheduler() {
		return mScheduler;
	}

private:
	static void readCallback(void* arg, int fd);
	void handleRead(int fd);
	static void disConnectCallback(void* arg,int clientFd);
	void handleDisConnect(int clientFd);
	static void closeConnectCallback(void* arg, int fd);
	void handleCloseConnect();
	static void sendCallback(void* arg, int fd, std::string mess);
	void sendtoAllServer(std::string mess);
	void sendMess(int fd, std::string mess);
	void serverConnectInit(TRServer*);

	void reConnect();
	static void reConnectTimeoutCallback(void* arg, int fd);
	static void reConnectTaskCallback(void* arg);
	static void readOfflineMessTaskCallback(void* arg);
	static void sendOffLineMessCallback(void* arg, int fd);
	void handleOffLineMess();
	

public:
	void handleMess(void* arg, std::string mess);
	
	

private:
	TRServer* mTRServer;
	EventScheduler* mScheduler;
	ThreadPool* mThreadPool;
	Task* mReConnectTask;
	Task* mReadOfflineMessTask;
	std::map<int, ISRConnection*>mConnectMap;
	std::map<int, ISRConnection*>mServerMap;
	std::vector<int> mDisConnectList;
	TriggerEvent* mCloseTriggerEvent;
	std::mutex mMtx;
	int send06Flag = 0;

	int mFd;
	IOEvent* mAcceptIOEvent;
	InetAddress mAddr;
	TimerEvent* mReconnectServerTimerEvent;
	TimerEvent* mSendOffLineMessTimerEvent;
	std::string mOffLinePath = "/home/ubuntu/projects/TRLIB_ISR/message.txt";
	std::queue<std::string> mOffLineMessQue;
};

#endif // !TRLIB_NETSERVER_H
