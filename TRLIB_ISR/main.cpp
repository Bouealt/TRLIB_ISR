#include<stdio.h>
#include<iostream>
#include"../Base/EventScheduler.h"
#include"../Base/ThreadPool.h"
#include"InetAddress.h"
#include"ISRServer.h"

InetAddress isrWifiAddr("192.168.3.1", 1234, "isrWifi");//isr wifi地址
InetAddress isrLanAddr("192.168.2.1", 2234, "isrLan");//isr lan口地址

InetAddress serverAddr("tstit.x3322.net", 8080, "Server");//实验室本地服务器地址
InetAddress aliyunAddr("g2020.top", 8888, "Aliyun");//阿里云服务器地址

int main() {
	EventScheduler* scheduler = EventScheduler::createNew(EventScheduler::POLLER_EPOLL);	
	ThreadPool* threadPool = ThreadPool::createNew(1);
	std::vector<InetAddress> serverAddrs = { serverAddr, aliyunAddr };
	ISRServer* isrServer = ISRServer::createNew(scheduler, threadPool, isrWifiAddr, isrLanAddr, serverAddrs);
	isrServer->start();
	scheduler->loop();//处理事件
	return 0;
}