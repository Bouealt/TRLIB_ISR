#ifndef TRLIB_SELECTPOLLER_H
#define TRLIB_SELECTPOLLER_H
#include"Poller.h"
#include<vector>

class SelectPoller : public Poller {
public:
	SelectPoller();
	~SelectPoller();

	static SelectPoller* createNew();

	bool addIOEvent(IOEvent* event);
	bool updateIOEvent(IOEvent* event);
	bool removeIOEvent(IOEvent* event);
	void handleEvent();

private:
	fd_set mReadSet;
	fd_set mWriteSet;
	fd_set mExceptionSet;
	int mMaxNumSockets;
	std::vector<IOEvent*> mIOEvents;
};


#endif 