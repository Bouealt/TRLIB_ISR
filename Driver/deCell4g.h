#ifndef TRLIB_DECELL4G_H
#define TRLIB_DECELL4G_H


namespace deCell {
	int cellWanDetect(char* netName);
	int cell4gDetect(char* netName);
	int ipPortCheck(const char* web, int remotePort);
	char* getIp();
	int pppInit(char* netName);
}



#endif // !TRLIB_DECELL4G_H
