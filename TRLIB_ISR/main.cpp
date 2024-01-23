#include<stdio.h>
#include<iostream>
#include"../Base/EventScheduler.h"
#include"InetAddress.h"
#include"ISRServer.h"
#include"../Base/ThreadPool.h"

InetAddress aliyunAddr("192.168.110.1", 4001, "Aliyun");//阿里云服务器地址
InetAddress serverAddr("192.168.110.1", 4444, "Server");

InetAddress mAddr("192.168.110.128", 8555, "myaddr");//本地端口与地址

int main() {
	EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_EPOLL);//调度器
	ThreadPool* threadPool = ThreadPool::createNew(1);//线程池
	std::vector<InetAddress> serverAddrs = { aliyunAddr };
	ISRServer* isrServer = ISRServer::createNew(scheduler, threadPool, serverAddrs, mAddr);//创建本地isr服务器
	isrServer->start();
	scheduler->loop();//处理事件
	return 0;
}