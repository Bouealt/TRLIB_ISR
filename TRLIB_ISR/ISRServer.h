#ifndef TRLIB_ISRSERVER_H
#define TRLIB_ISRSERVER_H
#include <mutex>
#include "../Base/Event.h"
#include "../Base/EventScheduler.h"
#include "../Base/ThreadPool.h"
#include "ISRConnection.h"
#include "InetAddress.h"
#include "Device.h"
#include "TRServer.h"
#include <queue>
class ISRServer
{
public:
	static std::unique_ptr<ISRServer> createNew(EventScheduler *scheduler, ThreadPool *threadPool, InetAddress &wifiAddr, InetAddress &lanAddr, std::vector<InetAddress> serverAddrs);
	ISRServer(EventScheduler *scheduler, ThreadPool *threadPool, InetAddress &wifiAddr, InetAddress &lanAddr, std::vector<InetAddress> serverAddrs);
	~ISRServer();
	void start();
	EventScheduler *getScheduler()
	{
		return mScheduler;
	}

private:
	static void bluetoothReadCallback(void *arg, int fd);
	void handleBluetoothRead(int fd);

	static void readCallback(void *arg, int fd);
	void handleRead(int fd);
	void handleBlueToothData(const char *data, int length);

	static void disConnectCallback(void *arg, int clientFd);
	void handleDisConnect(int clientFd);
	static void closeConnectCallback(void *arg, int fd);
	void handleCloseConnect();
	static void sendCallback(void *arg, int fd, std::string mess);
	void sendtoAllServer(std::string mess);
	void sendMess(int fd, std::string mess);
	void deviceConnectInit(std::unique_ptr<Device> &device);
	void serverConnectInit(std::unique_ptr<TRServer> &server);
	void regist();

	static void timeOutCallback03(void *arg, int fd);
	static void timeOutCallback22(void *arg, int fd);
	static void timeOutCallbackppp(void *arg, int fd);
	static void reConnectTaskCallback(void *arg);
	static void readOfflineMessTaskCallback(void *arg);

	void reConnect();
	static void reConnectTimeoutCallback(void *arg, int fd);
	static void sendOffLineMessCallback(void *arg, int fd);
	void handleOffLineMess();

public:
	void handle01Mess(void *arg, std::string sapMac); // sap向isr注册
	void handle02Mess(void *arg, std::string sapMac); // sap确认注册
	void handle03Mess(int fd);						  // isr消息请求
	void handle05Mess(void *arg, std::string sapMac); // sap向服务器注册
	void handle06Mess(void *arg, std::string mess);	  // sap数据
	void handle10Mess(void *arg, std::string mess);	  // 服务器时间戳
	void handle17Mess(void *arg, std::string sapMac); // isr信息下发
	void handle20Mess(void *arg, std::string sapMac); // isr时间戳
	void handle21Mess(void *arg, std::string mess);	  // sap缓存数据
	void handle22Mess();							  // isr心跳

private:
	std::unique_ptr<Device> mDevice;	 // 连接设备
	std::unique_ptr<TRServer> mTRServer; // 服务器
	ThreadPool *mThreadPool;
	EventScheduler *mScheduler;
	Task *mReConnectTask;		// 重连任务
	Task *mReadOfflineMessTask; // 读取离线数据任务
	IOEvent *mWifiAcceptIOEvent;
	IOEvent *mLanAcceptIOEvent;
	IOEvent *mBlueToothAcceptIOEvent;
	std::map<int, ISRConnection *> mConnectMap;		   // 所有连接
	std::map<int, ISRConnection *> mServerMap;		   // 服务器连接
	std::map<std::string, ISRConnection *> mSAPMacMap; // sap连接
	std::vector<int> mDisConnectList;
	TriggerEvent *mCloseTriggerEvent;
	std::mutex mMtx;
	bool isSetTime = false;
	int send06Flag = 0;

	TimerEvent *mReconnectServerTimerEvent;
	TimerEvent *mSendOffLineMessTimerEvent;
	std::string mOffLinePath = "/home/root/g2020/message.txt"; // 离线数据保存路径
	std::queue<std::string> mOffLineMessQue;				   // 离线数据缓存队列
};

#endif // !TRLIB_NETSERVER_H
