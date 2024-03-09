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
	TRServer* mTRServer;	//服务器连接
	EventScheduler* mScheduler;	//事件调度器
	ThreadPool* mThreadPool;	//线程池
	Task* mReConnectTask;	//重连任务
	Task* mReadOfflineMessTask;	//读离线消息任务
	std::map<int, ISRConnection*>mConnectMap;	//连接的fd和连接的映射
	std::map<int, ISRConnection*>mServerMap;	//服务器fd和连接的映射
	std::vector<int> mDisConnectList;	//断开连接的fd列表
	TriggerEvent* mCloseTriggerEvent;	//关闭连接触发事件
	std::mutex mMtx;	//互斥锁
	int send06Flag = 0;	//发送06标志

	int mFd;	
	IOEvent* mAcceptIOEvent;	//接受连接事件
	InetAddress mAddr;	//服务器地址
	TimerEvent* mReconnectServerTimerEvent;	//重连服务器定时器
	TimerEvent* mSendOffLineMessTimerEvent;	//发送离线消息定时器
	std::string mOffLinePath = "/home/ubuntu/projects/TRLIB_ISR/message.txt";	//离线消息文件路径
	std::queue<std::string> mOffLineMessQue;	//离线消息队列
};

#endif // !TRLIB_NETSERVER_H
