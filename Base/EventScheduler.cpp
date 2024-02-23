#include"EventScheduler.h"
#include"Log.h"

EventScheduler* EventScheduler::createNew(PollerType type) {
	return new EventScheduler(type);
}

EventScheduler::EventScheduler(PollerType type) {
	switch (type) {
		case POLLER_SELECT:
			mPoller = SelectPoller::createNew();
			break;
		case POLLER_EPOLL:
			mPoller = EpollPoller::createNew();
			break;
		default:
			LOGE("PollerType error");
			_Exit(-1);
			break;
	}

	mTimerManager = TimerManager::createNew(this);
}

EventScheduler::~EventScheduler() {
	delete mPoller;
	delete mTimerManager;
}

bool EventScheduler::addIOEvent(IOEvent* event) {
	return updateIOEvent(event);
}

bool EventScheduler::updateIOEvent(IOEvent* event) {
	return mPoller->updateIOEvent(event);
}

bool EventScheduler::removeIOEvent(IOEvent* event) {
	return mPoller->removeIOEvent(event);
}

void EventScheduler::loop() {

	while (true)
	{
		handleTriggerEvent();
		mPoller->handleEvent();
	}
}

bool EventScheduler::addTriggerEvent(TriggerEvent* event) {
	mTriggerEvents.push_back(event);	//将触发事件添加到mTriggerEvents中
	return true;
}

Timer::TimerId EventScheduler::addTimerEventRunEvery(TimerEvent* event, Timer::TimeInterval interval)	//添加定时器事件
{
	LOGI("add timerEvent run every time. Interval = %d s", interval / 1000);
	Timer::TimeStamp timeStamp = Timer::getCurTime();
	timeStamp += interval;	//时间戳加上时间间隔
	return mTimerManager->addTimer(event, timeStamp, interval);	//添加定时器事件
}

void EventScheduler::setTimerInterval(TimerEvent* event, Timer::TimeInterval interval) {
	LOGI("set timerEvent interval = %d s", interval / 1000);
	mTimerManager->setTimerInterval(event, interval);
}

bool EventScheduler::removeTimerEvent(Timer::TimerId timerId)
{
	return mTimerManager->removeTimer(timerId);
}

void EventScheduler::handleTriggerEvent() {
	if (mTriggerEvents.empty())return;
	for (auto it : mTriggerEvents) {
		it->handleEvent();
	}
	mTriggerEvents.clear();
}

Poller* EventScheduler::getPoller() {
	return mPoller;
}