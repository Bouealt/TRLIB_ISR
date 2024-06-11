#include"Device.h"
#include"../Driver/usbctl.h"
#include"../Base/SocketsOps.h"
#include"../Driver/deCell4g.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <string.h>

Device* Device::createNew(InetAddress& wifiAddress, InetAddress& lanAddress) {
	return new Device(wifiAddress, lanAddress);
}

Device::Device(InetAddress& wifiAddress, InetAddress& lanAddress):
        mWifiAddress(wifiAddress),
        mLanAddress(lanAddress)
{
	mLoraFd = openLora();
	//sleep(2);
    mWifiFd = openWifi();
    mBlueToothFd = openBlueTooth();
    mLanFd = openLan();
    std::string s = getNetId();
    if (s != "") {
        mNetId = s;
    }
    s = getMacId();
    if (s != "") {
        mMacId = s;
    }
    s = deCell::getIp();
    if (s != "") {
        mIp = s;
    }
    system("echo 0 > /sys/class/leds/green/brightness");	//初始化绿灯
}

Device::~Device() {
    if (-1 != mLoraFd)sockets::close(mLoraFd);
    if (-1 != mWifiFd)sockets::close(mWifiFd);
    if (-1 != mBlueToothFd)sockets::close(mBlueToothFd);
    if (-1 != mLanFd)sockets::close(mLanFd);
}

int Device::openLora() {
	int loraFd = usbctl::openPort(0);
	if (-1 == loraFd) {
        std::cout << "lora open error" << std::endl;
		return -1;
	}
    usbctl::setOpt(loraFd, 115200, 8, 'n', 1);
    deviceNum += 1;
	mDeviceFd[loraFd] = "loraFd";
    std::cout << "Init success: lan, Device dscriptor = " << loraFd << std::endl;
	return loraFd;
}

int Device::openWifi() {
    /**wifi设备初始化**//*驱动不对或者版本不对*/
    //在执行hostapd和udhcpd之前，应该先将现有的进程杀掉
    system("killall hostapd");
    //sleep(2);
    system("killall udhcpd");
    //sleep(2);
    //首先开启wlan0
    //打开wlan0，并开启AP模式
    system("hostapd -B /home/root/g2020/program/software/wifi/hostapd.conf &");//-----------
    //sleep(5);
    //启动udhcpd 进行DHCP自动分配ip
    //首先配置wlan0的ip和netmask，不然会报错
    system("ifconfig wlan0 192.168.3.1 netmask 255.255.255.0");
    //sleep(2);
    system("udhcpd -f /home/root/g2020/program/software/wifi/udhcpd.conf &");
    //sleep(2);
    int wifiFd = sockets::createTcpSock();
    if (wifiFd < 0) {
        std::cout << "wifiFd create error" << std::endl;
        return -1;
    }
    sockets::setReuseAddr(mWifiFd, 1);
    int ret = bind(wifiFd, mWifiAddress.getAddr(), sizeof(struct sockaddr));
    if (ret < 0) {
        std::cout << "wifi bind error" << std::endl;
        return -1;
    }
    deviceNum += 1;
    mDeviceFd[wifiFd] = "wifiFd";
    std::cout << "Init success: wifi, device descriptor = " << wifiFd << std::endl;
    return wifiFd;
}

int Device::openBlueTooth() {
    // 未测试------
    int fd = -1, ret;
    char *p;
    char buf[33];
    struct timeval timeout;
    fd_set fdset;

    // 遍历com口
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int i = 5;
    while (i > 0) {
        fd = usbctl::openPort(2);
        if (fd == -1) {
            i--;
            continue;
        } else {
            break;
        }
    }

    if (fd != -1) {
        uint8_t RST[] = "AT+Z=1\r\n";              // 重启
        uint8_t OPEN_VISUAL[] = "AT+SPP=1\r\n";    // 设置可见
        uint8_t SET_DNAME[] = "AT+DNAME=g2020\r\n"; // 设置蓝牙名称为 g2020
        // 配置USB端口
        ret = usbctl::setOpt(fd, 115200, 8, 'n', 1);

        // 发送重启命令
        usbctl::writePort(fd, RST, strlen((const char*)RST));
        sleep(1);

        // 监听事件 1 秒钟 超时跳过
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        ret = select(fd + 1, &fdset, NULL, NULL, &timeout);
        if (ret > 0 && FD_ISSET(fd, &fdset)) {
            ret = usbctl::readPort(fd, buf, sizeof(buf) - 1);
            if (ret > 0) {
                buf[ret] = '\0';  // 确保字符串以 '\0' 结尾
                p = strstr(buf, "OK");
                if (p != NULL) {
                    // 设置蓝牙可见
                    usbctl::writePort(fd, OPEN_VISUAL, strlen((const char*)OPEN_VISUAL));
                    sleep(2);

                    // 监听事件 1 秒钟 超时跳过
                    FD_ZERO(&fdset);
                    FD_SET(fd, &fdset);
                    timeout.tv_sec = 1;
                    timeout.tv_usec = 0;
                    ret = select(fd + 1, &fdset, NULL, NULL, &timeout);
                    if (ret > 0 && FD_ISSET(fd, &fdset)) {
                        ret = usbctl::readPort(fd, buf, sizeof(buf) - 1);
                        if (ret > 0) {
                            buf[ret] = '\0';  // 确保字符串以 '\0' 结尾
                            p = strstr(buf, "OK");
                            if (p != NULL) {
                                // 设置蓝牙名称
                                usbctl::writePort(fd, SET_DNAME, strlen((const char*)SET_DNAME));
                                sleep(1);

                                // 监听事件 1 秒钟 超时跳过
                                FD_ZERO(&fdset);
                                FD_SET(fd, &fdset);
                                timeout.tv_sec = 1;
                                timeout.tv_usec = 0;
                                ret = select(fd + 1, &fdset, NULL, NULL, &timeout);
                                if (ret > 0 && FD_ISSET(fd, &fdset)) {
                                    ret = usbctl::readPort(fd, buf, sizeof(buf) - 1);
                                    if (ret > 0) {
                                        buf[ret] = '\0';  // 确保字符串以 '\0' 结尾
                                        p = strstr(buf, "OK");
                                        if (p != NULL) {
                                            return fd;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        close(fd);
    }
    return -1;
}


int Device::openLan() {
    system("ifconfig eth0 192.168.2.1 netmask 255.255.255.0");
    int lanFd = sockets::createTcpSock();
    if (lanFd < 0) {
        std::cout << "lanFd create error" << std::endl;
        return -1;
    }
    sockets::setReuseAddr(lanFd, 1);
    int ret = bind(lanFd, mLanAddress.getAddr(), sizeof(struct sockaddr));
    if (ret < 0) {
        std::cout << "Lan bind error" << std::endl;
        return -1;
    }
    deviceNum += 1;
    mDeviceFd[lanFd] = "lanFd";
    std::cout << "Init success: lan, device descriptor = " << lanFd << std::endl;
    return lanFd;
}

std::string Device::getNetId() {
    std::ifstream ifs;
    std::string netId;
    ifs.open("/home/root/g2020/program/device_number/de_number.txt", std::ios::in);
    if (ifs) {
        //文件打开成功
        getline(ifs, netId);
        return netId;
    }
    std::cout << "mNetId get error" << std::endl;
}

std::string Device::getMacId() {
    std::ifstream ifs;
    std::string macId;
    ifs.open("/home/root/g2020/program/device_number/mac_number.txt", std::ios::in);
    if (ifs) {
        //文件打开成功
        getline(ifs, macId);
        return macId;
    }
    std::cout << "mMacId get error" << std::endl;
}