#include"EventScheduler.h"

EventScheduler* EventScheduler::createNew(PollerType type) {
	return new EventScheduler(type);
}

EventScheduler::EventScheduler(PollerType type) {
	switch (type) {
		case POLLER_SELECT:
			mPoller = SelectPoller::createNew();
			break;
		case POLLER_EPOLL:
			mPoller = EpollPoller::createNew();		//mPoller = EpollPoller::createNew();
			break;
		default:
			std::cout << "PollerType error" << std::endl;
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
	mTriggerEvents.push_back(event);
	return true;
}

Timer::TimerId EventScheduler::addTimerEventRunEvery(TimerEvent* event, Timer::TimeInterval interval)
{
	std::cout << "add timerEvent " << event->getName() << " run every time, Interval = " << interval/1000 << "s" << std::endl;
	Timer::TimeStamp timeStamp = Timer::getCurTime();
	timeStamp += interval;
	return mTimerManager->addTimer(event, timeStamp, interval);
}  
void EventScheduler::handleTriggerEvent() {
	printf("handleTriggerEvent\n");
	if (mTriggerEvents.empty())return;
	for (auto it : mTriggerEvents) {
		it->handleEvent();
	}
	mTriggerEvents.clear();
}

Poller* EventScheduler::getPoller() {
	return mPoller;
}