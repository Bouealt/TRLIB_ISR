#ifndef TRLIB_SELECTPOLLER_H
#define TRLIB_SELECTPOLLER_H
#include"Poller.h"
#include"Log.h"
#include<vector>

class SelectPoller : public Poller {
public:
	SelectPoller();
	~SelectPoller();

	static SelectPoller* createNew();	//静态工厂模式

	bool addIOEvent(IOEvent* event);
	bool updateIOEvent(IOEvent* event);
	bool removeIOEvent(IOEvent* event);
	void handleEvent();

private:
	fd_set mReadSet;	//读集合
	fd_set mWriteSet;	//写集合
	fd_set mExceptionSet;	//异常集合
	int mMaxNumSockets;	//Socket的最大数量
	std::vector<IOEvent*> mIOEvents;	//IO事件数组
};


#endif 