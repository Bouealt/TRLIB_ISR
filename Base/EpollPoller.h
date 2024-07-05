#ifndef TRLIB_EPOLLPOLLER_H
#define TRLIB_EPOLLPOLLER_H
#include "Poller.h"
#include "sys/epoll.h"
#include <vector>

class EpollPoller : public Poller
{
public:
	EpollPoller();

	static EpollPoller *createNew();

	bool addIOEvent(IOEvent *event);
	bool updateIOEvent(IOEvent *event);
	bool removeIOEvent(IOEvent *event);
	void handleEvent();

private:
	int mFd;
	int mMaxNumSockets;
	std::vector<IOEvent *> mIOEvents;
	epoll_event mEv;
};

#endif