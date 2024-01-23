#include"Event.h"
#include"Log.h"

TriggerEvent::TriggerEvent(void* arg,int fd, std::string mess):mArg(arg),mFd(fd),mMess(mess) {
	//std::cout << "TriggerEvent start\n" << std::endl;
}

TriggerEvent::~TriggerEvent() {
	//std::cout << "TriggerEvent end\n" << std::endl;
}

TriggerEvent* TriggerEvent::createNew(void* arg, int fd, std::string mess) {
	return new TriggerEvent(arg, fd, mess);
}

TriggerEvent* TriggerEvent::createNew() {
	return new TriggerEvent(NULL, -1, "");
}

void TriggerEvent::handleEvent() {
	if (mTriggerCallback) { mTriggerCallback(mArg, mFd); }
	if (mSendCallback) { mSendCallback(mArg, mFd, mMess);  }
}


TimerEvent::TimerEvent(void* arg,int fd):
	mArg(arg),
	mFd(fd),
	mTimeoutCallback(NULL),
	mIsStop(true),
	mExeTimes(0)
{
	std::cout << "TimerEvent start" << std::endl;
}

TimerEvent::~TimerEvent() {
	//std::cout << "TimerEvent end" << std::endl;
}

TimerEvent* TimerEvent::createNew(void* arg, int fd) {
	LOGI("TimerEvent create");
	return new TimerEvent(arg, fd);
}

TimerEvent* TimerEvent::createNew() {
	return new TimerEvent(NULL, -1);
}

bool TimerEvent::handleEvent() {
	if (mIsStop) {
		return mIsStop;
	}
	if (mTimeoutCallback) { mTimeoutCallback(mArg, mFd); }
	return mIsStop;
}

void TimerEvent::stop() {
	mIsStop = true;
}

void TimerEvent::start() {
	mIsStop = false;
}

bool TimerEvent::isStop() {
	return mIsStop;
}

void TimerEvent::addExeTimes()
{
	mExeTimes+=1;
}

void TimerEvent::setTimerId(uint32_t timerId)
{
	mTimerId = timerId;
}

uint32_t TimerEvent::getTimerId()
{
	return mTimerId;
}

IOEvent::IOEvent(int fd, void* arg):mFd(fd),mArg(arg) {
	//std::cout << "fd = " << mFd << std::endl;
}

IOEvent::~IOEvent() {
	//std::cout << "fd = " << mFd << std::endl;
}

IOEvent* IOEvent::createNew(int fd, void* arg) {
	return new IOEvent(fd, arg);
}

//IOEvent* IOEvent::createNew(int fd) {
//	return new IOEvent(fd, NULL);
//}

void IOEvent::handleEvent() {
	if (mReadCallback && (mEvent & EVENT_READ))
	{
		mReadCallback(mArg, mFd);
	}
	if (mWriteCallback && (mEvent & EVENT_WRITE))
	{
		mWriteCallback(mArg, mFd);
	}
}