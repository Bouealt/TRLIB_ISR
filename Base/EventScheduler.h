#ifndef TRLIB_EventScheduler_H
#define TRLIB_EventScheduler_H

#include"vector"
#include"Event.h"
#include"SelectPoller.h"
#include"EpollPoller.h"
#include"Timer.h"

class Poller;//类的前向声明

class EventScheduler {
public:
	enum PollerType {
		POLLER_SELECT,
		POLLER_POLL,
		POLLER_EPOLL
	};

	static EventScheduler* createNew(PollerType type);

	EventScheduler(PollerType type);
	~EventScheduler();

	bool addIOEvent(IOEvent* event);
	bool updateIOEvent(IOEvent* event);
	bool removeIOEvent(IOEvent* event);
	void loop();
	bool addTriggerEvent(TriggerEvent* event);
	Timer::TimerId addTimerEventRunEvery(TimerEvent* event, Timer::TimeInterval interval);
	bool removeTimerEvent(Timer::TimerId timeId);
	Poller* getPoller();
	void setTimerInterval(TimerEvent* event, Timer::TimeInterval interval);
	

private:
	void handleTriggerEvent();

private:
	Poller* mPoller;
	std::vector<TriggerEvent*> mTriggerEvents;
	TimerManager* mTimerManager;

};

#endif