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
	int mLoraFd = -1;	//lora套接字fd
	int mWifiFd = -1;	//wifi套接字fd
	int mBlueToothFd = -1;	//蓝牙fd
	int mLanFd = -1;	//lan套接字fd
	int deviceNum = 0;	//设备数量
	std::string memR = "85";//目前没用上
	std::string cpuR = "30";//目前没用上
	std::string mGps = "E2923N10636";//目前没用上，暂时写死
	std::string mNetId = "0000";	//本地网卡地址
	std::string mMacId = "ISR0112912310101";	//本地Mac地址
	std::string mIp;	//本地IP地址
	InetAddress mWifiAddress;	//wifi地址
	InetAddress mLanAddress;	//lan地址
	std::map<int, std::string>mDeviceFd;	//设备fd映射map
	
};
#endif // !TRLIB_DEVICE_H
