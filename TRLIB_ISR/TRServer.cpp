#include"TRServer.h"
#include"../Driver/deCell4g.h"
#include<iostream>
#include<unistd.h>

TRServer* TRServer::createNew(std::vector<InetAddress> serverAddrs) {
	return new TRServer(serverAddrs);
}

TRServer::TRServer(std::vector<InetAddress> serverAddrs) :
	mServerAddrs(serverAddrs)
{
	wanInit();
	pppInit();
	mServerNum = serversConnect(mServerAddrs);
	/*serverConnect();
	aliyunConnect();*/
}

void TRServer::wanInit(){
	system("/home/root/g2020/mokuai/4g/ppp/ppp/quectel-ppp-kill");
	int wanFlag = deCell::cellWanDetect("eth1");	//检测网络接口并获取其IP地址
	if (-1 == wanFlag) {
		std::cout << "WAN error" << std::endl;
	}
	std::cout << "WAN模式启动" << std::endl;
	//sleep(4);
}

//void TRServer::serverConnect() {
//	//mServerFd = deCell::ipPortCheck(mServerAddr.getIp().c_str(), mServerAddr.getPort());//isr先判断能否通过Lan连接服务器，如果不能，则ppp拨号后连接服务器
//	if (-1 == mServerFd) {
//		LOGE("Init error, ppp start");
//		if (-1 == deCell::pppInit("ppp0")) {
//			LOGI("ppp error");
//		}
//		else {
//			LOGI("ppp success");
//		}
//		mServerFd = deCell::ipPortCheck(mServerAddr.getIp().c_str(), mServerAddr.getPort());
//	}
//	if (-1 == mServerFd) {
//		LOGI("connect server error");
//	}
//	else {
//		mServerNum += 1;
//		LOGI("connect server success");
//	}
//}

int TRServer::serversConnect(std::vector<InetAddress> addrs) {
	mDisconnectServerAddrs.clear();
	int serverNum = 0;
	for (auto it : addrs) {
		int fd = deCell::ipPortCheck(it.getIp().c_str(), it.getPort());
		std::string serverName = it.getName();
		if (-1 == fd) {
			std::cout << "connect " << serverName << " error" << std::endl;
			mDisconnectServerAddrs.push_back(it);
		}
		else {
			std::cout << "connect " << serverName << " success" << std::endl;
			mServerMap.insert(std::make_pair(fd, it));
			serverNum++;
		}
	}
	return serverNum;
}

void TRServer::pppInit() {
	std::cout << "ppp start" << std::endl;
	if (-1 == deCell::pppInit("ppp0")) {
		std::cout << "ppp error" << std::endl;
		  // 等待2分钟后重启设备
		std::cout<< "Will reboot the device in two minutes" <<std::endl;
        sleep(120);  // 120秒，即2分钟
        std::cout << "Rebooting the system..." << std::endl;
        system("reboot");  // 执行重启命令
	}
	else {
		std::cout << "ppp success" << std::endl;
	}
}

int TRServer::reConnect() {
	mServerNum += serversConnect(mDisconnectServerAddrs);
	return mServerAddrs.size() == mServerNum;
}


void TRServer::disConnect(int fd) {
	// std::cout << mServerMap[fd].getName() << " disconnect" << std::endl;
	// mServerNum--;
	// mDisconnectServerAddrs.push_back(mServerMap[fd]);
	// mServerMap.erase(fd);
	try {
        if (mServerMap.find(fd) == mServerMap.end()) {
            throw std::runtime_error("fd not found in mServerMap");
        }
        std::cout << mServerMap[fd].getName() << " disconnect" << std::endl;
        mServerNum--;
        mDisconnectServerAddrs.push_back(mServerMap[fd]);
        mServerMap.erase(fd);
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}