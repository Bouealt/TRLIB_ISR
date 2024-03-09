#ifndef TRLIB_EventScheduler_H
#define TRLIB_EventScheduler_H

#include"vector"
#include"Event.h"
#include"SelectPoller.h"
#include"EpollPoller.h"
#include"Timer.h"

class Poller;//类的前向声明

class EventScheduler {	//事件调度器
public:
	//Poller类型
	enum PollerType {
		POLLER_SELECT,
		POLLER_POLL,
		POLLER_EPOLL
	};

	static EventScheduler* createNew(PollerType type);	//静态工厂模式

	EventScheduler(PollerType type);
	~EventScheduler();

	bool addIOEvent(IOEvent* event);	//添加IO事件
	bool updateIOEvent(IOEvent* event);
	bool removeIOEvent(IOEvent* event);
	void loop();	//事件循环
	bool addTriggerEvent(TriggerEvent* event);	//添加触发事件
	Timer::TimerId addTimerEventRunEvery(TimerEvent* event, Timer::TimeInterval interval);	//添加定时器事件
	bool removeTimerEvent(Timer::TimerId timeId);
	Poller* getPoller();
	void setTimerInterval(TimerEvent* event, Timer::TimeInterval interval);
	

private:
	void handleTriggerEvent();

private:
	Poller* mPoller;	//IO事件分发器
	std::vector<TriggerEvent*> mTriggerEvents;	//触发事件容器
	TimerManager* mTimerManager;	//定时器管理器

};

#endif