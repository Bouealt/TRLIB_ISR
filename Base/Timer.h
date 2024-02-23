#ifndef TRLIB_TIMER_H
#define TRLIB_TIMER_H
#include<map>
#include<stdint.h>

//类的前向声明
class EventScheduler;
class Poller;
class TimerEvent;
class IOEvent;

class Timer {	//定时器
public:	
	typedef uint32_t TimerId;	//定时器id
	typedef int64_t TimeStamp;	//时间戳
	typedef uint32_t TimeInterval;	//时间间隔

	Timer();
	~Timer();

	static TimeStamp getCurTime();	//静态成员函数，可以在类外直接调用，不用创建对象
	static TimeStamp getCurTimeStamp();

private:
	friend class TimerManager;	//友元类，可以访问Timer的私有成员
	Timer(TimerEvent* event, TimeStamp stamp, TimeInterval interval, TimerId timeId);

private:
	bool handleEvent();

private:
	TimerEvent* mTimerEvent;	//定时器事件
	TimeStamp mTimeStamp;
	TimeInterval mTimeInterval;
	TimerId mTimerId;
	bool mRepeat;	//是否重复
};

class TimerManager {	//定时器管理器
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
