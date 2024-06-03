#include"deCell4g.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include"../Base/SocketsOps.h"

namespace deCell {
    char ipaddr[20] = {};
}


int deCell::cellWanDetect(char* netName)    //检测指定网口
{
    int sock_fd;
    struct sockaddr_in my_addr;
    struct ifreq ifr;//ifreq配合 ioctl  一起使用  获取网口的信息
    /**/ /* Get socket file descriptor */
    if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
    {
        std::cout << "Socket create error" << std::endl;
        return -1;
    }
    /**/ /* Get IP Address */
    strncpy(ifr.ifr_name, netName, IF_NAMESIZE);    
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    if (ioctl(sock_fd, SIOCGIFADDR, &ifr) < 0)//获取接口ip地址
    {
        std::cout << "No such device : " << netName << std::endl;
        close(sock_fd);     //关闭错误套接字
        return -1;
    }
    memcpy(&my_addr, &ifr.ifr_addr, sizeof(my_addr));
    strncpy(deCell::ipaddr, inet_ntoa(my_addr.sin_addr), 20);//inet_ntoa:将网络地址转换为“.”点隔得字符串格式
    printf("Network addresss: %s\r\n", deCell::ipaddr);
    if (strlen(deCell::ipaddr) == 0) {
        std::cout << "Ip is not vaild" << std::endl;
        close(sock_fd);      //确保关闭文件描述符
        return -1;
    }
    close(sock_fd);
    return 0;
}

int deCell::ipPortCheck(const char* web, int remotePort)
{
    int sockfd, ret;
    char host[1024];
    struct addrinfo hints;
    struct addrinfo* res, * res_p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;  //所有协议簇
    hints.ai_socktype = SOCK_STREAM; //数据流
    hints.ai_flags = AI_CANONNAME;  //请求主机的名字
    hints.ai_protocol = 0;    //所有协议
    ret = getaddrinfo(web, NULL, &hints, &res);     //IP，端口，要求，结果
    if (ret != 0) {
        return -1;
    }
    for (res_p = res; res_p != NULL; res_p = res_p->ai_next) {
        ret = getnameinfo(res_p->ai_addr, res_p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
        if (ret == 0) {
            printf("ip: %s\n", host);     //域名解析得到ip地址
        }
    }
    char* remote_ip = host;
    // LAN设备配置
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        return -1;
    }
    else
    {
        std::cout << "Sockfd = " << sockfd << std::endl;
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO|SO_SNDTIMEO, &tv, sizeof(tv));
        struct sockaddr_in svraddr; //绑定地址(ip和端口号)
        memset(&svraddr, 0, sizeof(svraddr));
        svraddr.sin_family = AF_INET;
        svraddr.sin_port = htons(remotePort);
        inet_pton(AF_INET, remote_ip, &svraddr.sin_addr);
        int ret = connect(sockfd, (struct sockaddr*)&svraddr, sizeof(svraddr));//可以换成非阻塞连接，之后添加定时事件判断是否连接服务器
        if (ret < 0)
        {
            close(sockfd);
            if (errno == EINPROGRESS) {
                std::cout << "Connect timeout" << std::endl;
            }
            return -1;
        }
        else {
            /*printf("Connect server success\r\n");*/
        }
    }
    return sockfd;
}

int deCell::cell4gDetect(char* netName) {   //检测ppp0网口
    system("route add -net 0.0.0.0 netmask 0.0.0.0 dev ppp0");  //添加默认路由，通过ppp0
    int sock_fd;
    struct sockaddr_in my_addr;
    struct ifreq ifr;

    /**/ /* Get socket file descriptor */
    if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
    {
        std::cout << "Socket create error" << std::endl;
        return -1;
    }
    /**/ /* Get IP Address */
    strncpy(ifr.ifr_name, netName, IF_NAMESIZE);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(sock_fd, SIOCGIFADDR, &ifr) < 0)
    {
        std::cout << "No such device " << netName << std::endl;
        close(sock_fd);  //关闭错误的套接字
        return -1;
    }
    memcpy(&my_addr, &ifr.ifr_addr, sizeof(my_addr));
    strncpy(ipaddr, inet_ntoa(my_addr.sin_addr), 20);
    std::cout << "Network address: " << ipaddr << std::endl;
    if (strlen(ipaddr) == 0) {
        std::cout << "Ip is not vaild" << std::endl;
        close(sock_fd);  //关闭错误的套接字
        return -1;
    }
    close(sock_fd);
    return 0;
}

int deCell::pppInit(char* netName) {
    int retry = 2;
    system("/home/root/g2020/mokuai/4g/ppp/ppp/quectel-ppp-kill");
    while (retry--) {
        system("/home/root/g2020/mokuai/4g/ppp/ppp/quectel-pppd.sh");
        sleep(2);
        if (0 == cell4gDetect(netName)) {
            return 0;
        }
    }
    system("/home/root/g2020/mokuai/4g/ppp/ppp/quectel-ppp-kill");
    return -1;
}

char* deCell::getIp() {
    return deCell::ipaddr;
}