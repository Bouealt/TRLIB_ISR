#ifndef TRLIB_EPOLLPOLLER_H
#define TRLIB_EPOLLPOLLER_H
#include"Poller.h"
#include"Log.h"
#include"sys/epoll.h"
#include<vector>

class EpollPoller : public Poller {
public:
	EpollPoller();
	~EpollPoller();

	static EpollPoller* createNew();	//静态工厂模式

	bool addIOEvent(IOEvent* event);
	bool updateIOEvent(IOEvent* event);
	bool removeIOEvent(IOEvent* event);
	void handleEvent();

private:
	int mFd;	//epoll的fd
	int mMaxNumSockets;	//Socket的最大数量
	std::vector<IOEvent*> mIOEvents;	//IO事件数组
	epoll_event mEv;	//epoll_event结构体
};	


#endif 