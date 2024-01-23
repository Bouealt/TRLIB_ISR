#ifndef TRLIB_TCPCONNECTION_H
#define TRLIB_TCPCONNECTION_H
#include"../Base/EventScheduler.h"
#include"../Base/Event.h"
#include"Buffer.h"
#include<string>
typedef void (*DisConnectCallback)(void* arg, int fd);

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
	static void readCallback(void*, int);
	static void writeCallback(void*, int);
	static void errorCallback(void*, int);

protected:
	int mFd;
	EventScheduler* mScheduler;
	DisConnectCallback mDisConnectCallback;
	IOEvent* mIOEvent;
	void* mArg;
	Buffer mInputBuffer;
	Buffer mOutputBuffer;
};

#endif // !
