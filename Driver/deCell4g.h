#ifndef TRLIB_DECELL4G_H
#define TRLIB_DECELL4G_H


namespace deCell {
	int cellWanDetect(char* netName);	//广域网
	int cell4gDetect(char* netName);	//4G
	int ipPortCheck(const char* web, int remotePort);	//检查端口是否开放
	char* getIp();
	int pppInit(char* netName);
}



#endif // !TRLIB_DECELL4G_H
