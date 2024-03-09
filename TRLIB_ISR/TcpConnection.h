#ifndef TRLIB_TCPCONNECTION_H
#define TRLIB_TCPCONNECTION_H
#include"../Base/EventScheduler.h"
#include"../Base/Event.h"
#include"Buffer.h"
#include<string>
typedef void (*DisConnectCallback)(void* arg, int fd);	//断开连接回调函数

class TcpConnection {
public:
	TcpConnection(EventScheduler* scheduler, int clientFd);
	virtual ~TcpConnection();

	void setDisConnectCallback(DisConnectCallback cb, void* arg);

protected:
	void enableReadHandling();
	void enableWriteHandling();
	void enableErrorHandling();
	void disableReadHandling();
	void disableWriteHandling();
	void disableErrorHandling();

	void handleRead();
	virtual void handleReadBytes();
	void handleWrite();
	virtual void handleWriteBytes();
	void handleError();
	
	void handleDisConnect();


private:
	static void readCallback(void*, int);	//回调函数
	static void writeCallback(void*, int);
	static void errorCallback(void*, int);

protected:
	int mFd;	//tcp连接的fd
	EventScheduler* mScheduler;	//事件调度器
	DisConnectCallback mDisConnectCallback;	//断开连接回调函数
	IOEvent* mIOEvent;	//IO事件
	void* mArg;	//回调函数参数
	Buffer mInputBuffer;	//输入缓冲区
	Buffer mOutputBuffer;	//输出缓冲区
};

#endif // !
