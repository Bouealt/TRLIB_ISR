#include "Timer.h"
#include <sys/timerfd.h>
#include <time.h>
#include <chrono>
#include "Event.h"
#include "EventScheduler.h"
#include "Poller.h"
#include "Log.h"

static bool timerFdSetTime(int fd, Timer::TimeStamp when, Timer::TimeInterval period) {
	struct itimerspec newVal;
	newVal.it_value.tv_sec = when / 1000;
	newVal.it_value.tv_nsec = when % 1000 * 1000000;
	newVal.it_interval.tv_sec = period / 1000;
	newVal.it_interval.tv_nsec = period % 1000 * 1000000;
	int oldVal = timerfd_settime(fd, TFD_TIMER_ABSTIME, &newVal, NULL);
	if (oldVal < 0) {
		return false;
	}
	return true;
}

Timer::Timer(TimerEvent* event, TimeStamp timeStamp, TimeInterval timeInterval,TimerId timerId) :
	mTimerEvent(event),
	mTimeStamp(timeStamp),
	mTimeInterval(timeInterval),
	mTimerId(timerId)
{
	if (timeStamp > 0) {
		mRepeat = true;
	}
	else {
		mRepeat = false;
	}
}

Timer::Timer() {

}


Timer::~Timer()
{

}

// 获取系统从启动到目前的毫秒数
Timer::TimeStamp Timer::getCurTime()
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return (now.tv_sec * 1000 + now.tv_nsec / 1000000);
}

Timer::TimeStamp Timer::getCurTimeStamp() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).
		count();
}

bool Timer::handleEvent()
{
	if (!mTimerEvent) {
		return false;
	}
	return mTimerEvent->handleEvent();
}

TimerManager* TimerManager::createNew(EventScheduler* scheduler)
{
	return new TimerManager(scheduler);
}

TimerManager::TimerManager(EventScheduler* scheduler) :
	mPoller(scheduler->getPoller()),
	mLastTimerId(0)
{
	mTimerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (mTimerFd < 0) {
		LOGE("Create TimerFd error");
		return;
	}
	LOGI("TimerFd = %d", mTimerFd);
	mTimerIOEvent = IOEvent::createNew(mTimerFd, this);
	mTimerIOEvent->setReadCallback(readCallback);
	mTimerIOEvent->enableReadHandling();
	modifyTimeout();
	mPoller->addIOEvent(mTimerIOEvent);
}

TimerManager::~TimerManager()
{
	mPoller->removeIOEvent(mTimerIOEvent);
	delete mTimerIOEvent;
}

Timer::TimerId TimerManager::addTimer(TimerEvent* event, Timer::TimeStamp timeStamp, Timer::TimeInterval timeInterval)
{
	++mLastTimerId;
	LOGI("LastTimerId = %d", mLastTimerId);
	Timer timer(event, timeStamp, timeInterval, mLastTimerId);
	mTimers.insert(std::make_pair(mLastTimerId, timer));
	mEvents.insert(std::make_pair(timeStamp, timer));
	event->setTimerId(mLastTimerId);
	modifyTimeout();
	return mLastTimerId;
}

bool TimerManager::removeTimer(Timer::TimerId timeId)
{
	if (mTimers.find(timeId) == mTimers.end()) {
		LOGE("Not find Timer which timeId = %d", timeId);
		return false;
	}
	Timer timer = mTimers[timeId];
	mEvents.erase(timer.mTimeStamp);
	mTimers.erase(timeId);
	modifyTimeout();
	return true;
}

void TimerManager::setTimerInterval(TimerEvent* event, Timer::TimeInterval interval)
{
	auto it = mTimers.find(event->getTimerId());
	if (it != mTimers.end()) {
		it->second.mTimeInterval = interval;
	}
}

void TimerManager::setTimerIntervalTwice(TimerEvent* event)
{
	auto it = mTimers.find(event->getTimerId());
	if (it != mTimers.end()) {
		it->second.mTimeInterval *= 2;
	}
}

void TimerManager::modifyTimeout()
{
	auto it = mEvents.begin();
	if (it != mEvents.end()) {
		Timer timer = it->second;
		timerFdSetTime(mTimerFd, timer.mTimeStamp, timer.mTimeInterval);
	}
	else {
		timerFdSetTime(mTimerFd, 0, 0);
	}
}

void TimerManager::readCallback(void* arg, int fd)
{
	TimerManager* timerManager = (TimerManager*)arg;
	timerManager->handleRead();
}

void TimerManager::handleRead()
{
	Timer::TimeStamp timeStamp = Timer::getCurTime();
	if (!mTimers.empty() && !mEvents.empty()) {
		auto it = mEvents.begin();
		Timer timer = it->second;
		int expire = timer.mTimeStamp - timeStamp;
		if (timeStamp > timer.mTimeStamp || expire == 0) {
			bool isStop = timer.handleEvent();
			mEvents.erase(it);
			it->second.mTimerEvent->addExeTimes();
			if (timer.mRepeat) {
				if (isStop) {
					mTimers.erase(timer.mTimerId);
				}
				else {
					timer.mTimeStamp = timeStamp + timer.mTimeInterval;
					mEvents.insert(std::make_pair(timer.mTimeStamp, timer));
				}
			}
			else {
				mTimers.erase(timer.mTimerId);
			}
		}
	}
	modifyTimeout();
}






