#include"EpollPoller.h"

EpollPoller::EpollPoller()
{
	mFd = epoll_create(100);	//创建epoll，100只是提示
}

EpollPoller::~EpollPoller() {

}

EpollPoller* EpollPoller::createNew() {
	return new EpollPoller();
}

bool EpollPoller::addIOEvent(IOEvent* event) {
	return updateIOEvent(event);
}

bool EpollPoller::updateIOEvent(IOEvent* event) {
	int fd = event->getFd();	//获取fd
	if (fd < 0) {	
		return false;
	}
	mEv.data.fd = fd;	//设置epoll_event结构体的fd
	mEv.events = EPOLLIN;
	if (mEventMap.find(fd) == mEventMap.end()) {	//如果fd不在mEventMap中
		mEventMap.insert(std::make_pair(fd, event));
	}
	epoll_ctl(mFd, EPOLL_CTL_ADD, fd, &mEv);	//将fd添加到epoll中

	if (mEventMap.empty()) {
		mMaxNumSockets = 0;
	}
	else {
		mMaxNumSockets = mEventMap.rbegin()->first + 1;	//获取mEventMap中最大的fd
	}
	return true;
}

bool EpollPoller::removeIOEvent(IOEvent* event) {
	int fd = event->getFd();
	if (fd < 0) {
		return false;
	}
	auto it = mEventMap.find(fd);
	if (it != mEventMap.end()){	//如果fd在mEventMap中
		mEventMap.erase(it);
	}
	if (mEventMap.empty()){
		mMaxNumSockets = 0;
	}
	else {
		mMaxNumSockets = mEventMap.rbegin()->first + 1;	//获取mEventMap中最大的fd，rbegin()返回的是反向迭代器
	}
	return true;
}

//处理事件
void EpollPoller::handleEvent() {	
	//epoll_event结构体，events包含EPOLLIN、EPOLLOUT、EPOLLPRI、EPOLLERR、EPOLLHUP等和data可以使fd或者ptr
	epoll_event events[mMaxNumSockets];	//存储epoll_wait返回的事件
	struct timeval timeout;		//epoll_wait的超时时间
	int rEvent = IOEvent::EVENT_NONE;	
	int ret = epoll_wait(mFd, events, mMaxNumSockets, 1000 * 1000);	//阻塞等待
	if (ret < 0) {
		return;
	}
	for (int i = 0; i < ret; i++) {
		int fd = events[i].data.fd;
		if (events[i].events & EPOLLIN) {	//读
			rEvent |= IOEvent::EVENT_READ;	//按位或
		}
		if (events[i].events & EPOLLOUT) {	//写
			rEvent |= IOEvent::EVENT_WRITE;	
		}
		if (events[i].events & EPOLLERR) {	//错误
			rEvent |= IOEvent::EVENT_ERROR;
		}

		if (rEvent != IOEvent::EVENT_NONE) {	//如果有事件发生
			mEventMap[fd]->setEvent(rEvent);	//设置事件
			mIOEvents.push_back(mEventMap[fd]);	//将事件加入到mIOEvents中
		}
	}
	for (auto& ioEvent : mIOEvents) {
		ioEvent->handleEvent();
	}
	mIOEvents.clear();
}