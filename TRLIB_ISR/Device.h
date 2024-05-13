#ifndef TRLIB_DEVICE_H
#define TRLIB_DEVICE_H
#include<map>
#include<string>
#include"InetAddress.h"

class Device {
public:
	static Device* createNew(InetAddress& wifiAddress, InetAddress& lanAddress);
	Device(InetAddress& wifiAddress, InetAddress& lanAddress);
	~Device();

private:
	int openLora();
	int openWifi();
	int openBlueTooth();
	int openLan();
	std::string getNetId();
	std::string getMacId();

public:
	int mLoraFd = -1;
	int mWifiFd = -1;
	int mBlueToothFd = -1;
	int mLanFd = -1;
	int deviceNum = 0;
	std::string memR = "85";//目前没用上
	std::string cpuR = "30";//目前没用上
	std::string mGps = "E2923N10636";//目前没用上，暂时写死
	std::string mNetId = "0000";
	std::string mMacId = "ISR0112912310101";
	std::string mIp;//本地4g网卡地址
	InetAddress mWifiAddress;
	InetAddress mLanAddress;
	std::map<int, std::string>mDeviceFd;
	
};
#endif // !TRLIB_DEVICE_H
