#include"Event.h"

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


TimerEvent::TimerEvent(void* arg, int fd, std::string name):
	mArg(arg),
	mFd(fd),
	mTimeoutCallback(NULL),
	mIsStop(true),
	mName(name)
{
	std::cout << mName << " TimerEvent create" << std::endl;
}

TimerEvent::~TimerEvent() {
	//std::cout << "TimerEvent end" << std::endl;
}

TimerEvent* TimerEvent::createNew(void* arg, int fd, std::string name) {
	return new TimerEvent(arg, fd, name);
}

bool TimerEvent::handleEvent() {
	if (mIsStop) {
		return mIsStop;
	}
	if (mTimeoutCallback) { mTimeoutCallback(mArg, mFd); }
	return mIsStop;
}

void TimerEvent::stop() {
	std::cout << mName << " stop" << std::endl;
	mIsStop = true;
}

void TimerEvent::start() {
	std::cout << mName << " start" << std::endl;
	mIsStop = false;
}

bool TimerEvent::isStop() {
	return mIsStop;
}

std::string TimerEvent::getName() {
	return mName;
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
	if (mReadCallback && (mEvent & EVENT_READ))	//检查是否有读事件
	{
		mReadCallback(mArg, mFd);
	}
	if (mWriteCallback && (mEvent & EVENT_WRITE))
	{
		mWriteCallback(mArg, mFd);
	}
}