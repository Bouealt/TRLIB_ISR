#include"EpollPoller.h"

EpollPoller::EpollPoller()
{
	mFd = epoll_create(100);
}

EpollPoller::~EpollPoller() {

}

EpollPoller* EpollPoller::createNew() {
	return new EpollPoller();
}

bool EpollPoller::addIOEvent(IOEvent* event) {
	return updateIOEvent(event);
}

//�? Linux 系统中，文件描述符的值通常�? 0 开始，最大值通常是打开文件的最大数量减一�?
bool EpollPoller::updateIOEvent(IOEvent* event) {
	int fd = event->getFd();
	if (fd < 0) {
		return false;
	}
	mEv.data.fd = fd;
	mEv.events = EPOLLIN;
	if (mEventMap.find(fd) == mEventMap.end()) {
		mEventMap.insert(std::make_pair(fd, event));
	}
	epoll_ctl(mFd, EPOLL_CTL_ADD, fd, &mEv);

	if (mEventMap.empty()) {
		mMaxNumSockets = 0;
	}
	else {
		mMaxNumSockets = mEventMap.rbegin()->first + 1;
	}
	return true;
}

bool EpollPoller::removeIOEvent(IOEvent *event)
{
	int fd = event->getFd();
	if (fd < 0)
	{
		return false;
	}
	auto it = mEventMap.find(fd);
	if (it != mEventMap.end())
	{
		mEventMap.erase(it);
		epoll_ctl(mFd, EPOLL_CTL_DEL, fd, nullptr); // ɾ���ļ�������*******
	}
	if (mEventMap.empty())
	{
		mMaxNumSockets = 0;
	}
	else
	{
		mMaxNumSockets = mEventMap.rbegin()->first + 1;
	}
	return true;
}

void EpollPoller::handleEvent() {
	epoll_event events[mMaxNumSockets];
	struct timeval timeout;
	int rEvent = IOEvent::EVENT_NONE;
	int ret = epoll_wait(mFd, events, mMaxNumSockets, 1000 * 1000);
	if (ret < 0) {
		return;
	}
	for (int i = 0; i < ret; i++) {
		int fd = events[i].data.fd;
		if (events[i].events & EPOLLIN) {	//events表示一组事�?
			rEvent |= IOEvent::EVENT_READ;
		}
		if (events[i].events & EPOLLOUT) {
			rEvent |= IOEvent::EVENT_WRITE;
		}
		if (events[i].events & EPOLLERR) {
			rEvent |= IOEvent::EVENT_ERROR;
		}

		if (rEvent != IOEvent::EVENT_NONE) {
			mEventMap[fd]->setEvent(rEvent);
			mIOEvents.push_back(mEventMap[fd]);
		}
	}
	for (auto& ioEvent : mIOEvents) {
		ioEvent->handleEvent();
	}
	mIOEvents.clear();
}