#include "usbctl.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "../Base/Log.h"
#include <sys/time.h>



// 串口/端口打开[非阻塞方式]
// @param
// @comport 需要打开的USB端口号 
// Lora标识0，tong_lora
// WIFI标识1，tong_wifi
// 蓝牙标识2，tong_bt
// 4G标识3，  tong_4g
// @return  打开失败返回< -1 >
//          打开成功返回对应的< 文件描述符fd>0 >
int usbctl::openPort(int comport) {
	int fd;
	switch (comport)
	{
	case 0:
		fd = open("/dev/tong_lora", O_RDWR | O_NOCTTY | O_NDELAY);  // 读写 | 不将此终端作为此进程的控制终端 | 非阻塞
		if (-1 == fd) {
			LOGI("Open lora fail");
			return -1;
		}
		break;
	case 1:
		fd = open("/dev/tong_wifi", O_RDWR | O_NOCTTY | O_NDELAY);
		if (-1 == fd) {
			LOGI("Open wifi fail");
			return -1;
		}
		break;
	case 2:
		fd = open("/dev/tong_bt", O_RDWR | O_NOCTTY | O_NDELAY);
		if (-1 == fd) {
			LOGI("Open bluetooth fail");
			return -1;
		}
		break;
	case 3:
		fd = open("/dev/tong_4g", O_RDWR | O_NOCTTY | O_NDELAY);
		if (-1 == fd) {
			LOGI("Open 4g fail");
			return -1;
		}
		break;
	default:
		LOGI("Port error");
		break;
	}
	//设置为非阻塞
	int fl = fcntl(fd, F_GETFL);
	if (fcntl(fd, F_SETFL, fl | O_NONBLOCK) < 0) {
		LOGE("Set nonblock fail");
	}
	return fd;
}

// 串口配置
// @param
// @fd      文件描述符fd
// @nSpeed  波特率选项< 2400 4800 9600 115200 460800 > 若不为上述选项，则自动设为9600
// @nBits   数据位数选项 < 7 8 >
// @nEvent  奇偶检验为选项 < o e n>
// @nStop   设置停止位 < 1 2 >
// @return  错误代码
int usbctl::setOpt(int fd, int nSpeed, int nBits, uint8_t nEvent, int nStop) {
    struct termios newtio, oldtio;
    // 保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息
    if (tcgetattr(fd, &oldtio) != 0)
    {
        perror("SetupSerial 1");
        printf("tcgetattr( fd,&oldtio) -> %d\n", tcgetattr(fd, &oldtio));
        return -1;
    }
    bzero(&newtio, sizeof(newtio)); // 类似memset
    // 设置字符大小
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;
    // 设置停止位
    switch (nBits)
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    default:
        LOGI("nBits error");
        break;
    }
    // 设置奇偶校验位
    switch (nEvent)
    {
    case 'o':
    case 'O': // 奇数
        newtio.c_cflag |= PARENB;   // 启用奇偶校验
        newtio.c_cflag |= PARODD;   // 奇数校验
        newtio.c_iflag |= (INPCK | ISTRIP); 
        break;
    case 'e':
    case 'E': // 偶数
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;  // 偶数校验
        break;
    case 'n':
    case 'N': // 无奇偶校验位
        newtio.c_cflag &= ~PARENB;
        break;
    default:
        LOGI("nEvent error");
        break;
    }
    // 设置波特率
    switch (nSpeed)
    {
    case 2400:
        cfsetispeed(&newtio, B2400);    // 设置输入波特率
        cfsetospeed(&newtio, B2400);    // 设置输出波特率
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 460800:
        cfsetispeed(&newtio, B460800);
        cfsetospeed(&newtio, B460800);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    // 设置停止位
    if (nStop == 1)
        newtio.c_cflag &= ~CSTOPB;  // 1位停止位
    else if (nStop == 2)
        newtio.c_cflag |= CSTOPB;   // 2位停止位
    // 设置等待时间和最小接收字符[该参数可根据需要调整，影响程序的阻塞特性]
    newtio.c_cc[VTIME] = 0; //VTIME指定读取第一个字符的等待时间，时间的单位为n*0.1s
    newtio.c_cc[VMIN] = 0;  //VMIN指定所要读取字符的最小数量
    // 处理未接收字符
    tcflush(fd, TCIFLUSH);
    // 激活新配置
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
    {
        LOGE("com set error");
        return -1;
    }
    return 0;
}


std::string usbctl::getTime() {
    struct timeval tv;
    struct tm* localTime;
    char timeStr[20];
    gettimeofday(&tv, NULL);
    localTime = localtime(&tv.tv_sec);
    strftime(timeStr, sizeof(timeStr), "%Y%m%d%H%M%S", localTime);
    return timeStr;
}