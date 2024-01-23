#ifndef TRLIB_TIMER_H
#define TRLIB_TIMER_H
#include<map>
#include<stdint.h>

class EventScheduler;
class Poller;
class TimerEvent;
class IOEvent;

class Timer {
public:
	typedef uint32_t TimerId;
	typedef int64_t TimeStamp;
	typedef uint32_t TimeInterval;

	Timer();
	~Timer();

	static TimeStamp getCurTime();
	static TimeStamp getCurTimeStamp();

private:
	friend class TimerManager;
	Timer(TimerEvent* event, TimeStamp stamp, TimeInterval interval, TimerId timeId);

private:
	bool handleEvent();

private:
	TimerEvent* mTimerEvent;
	TimeStamp mTimeStamp;
	TimeInterval mTimeInterval;
	TimerId mTimerId;
	bool mRepeat;
};

class TimerManager {
public:
	static TimerManager* createNew(EventScheduler* scheduler);
	TimerManager(EventScheduler* scheduler);
	~TimerManager();

	Timer::TimerId addTimer(TimerEvent* event, Timer::TimeStamp timeStamp, Timer::TimeInterval timeInterval);
	bool removeTimer(Timer::TimerId timeId);
	void setTimerInterval(TimerEvent* event, Timer::TimeInterval interval);
	void setTimerIntervalTwice(TimerEvent* event);

private:
	static void readCallback(void* arg, int fd);
	void handleRead();
	void modifyTimeout();

public:
	Poller* mPoller;
	std::map<Timer::TimerId, Timer> mTimers;
	std::multimap<Timer::TimeStamp, Timer> mEvents;
	Timer::TimerId mLastTimerId;
	int mTimerFd;
	IOEvent* mTimerIOEvent;
};


#endif // !TRLIB_TIMER_H
