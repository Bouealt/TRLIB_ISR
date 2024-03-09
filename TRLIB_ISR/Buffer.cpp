#include"Buffer.h"
#include "../Base/SocketsOps.h"
#include "../Base/Log.h"
#include<string.h>
#include <stdlib.h>
#include <algorithm>


Buffer::Buffer()
{
	mBufferSize = 0;
	mBuffer = (char*)malloc(RX_SIZE);	//分配内存
	memset(mBuffer, 0, RX_SIZE);	//初始化
}

Buffer::~Buffer() {	
	free(mBuffer);	//释放内存
}

int Buffer::read(int sockfd) {
	char RX_buf[RX_SIZE];
	memset(RX_buf, 0, sizeof(RX_buf));
	int n = ::read(sockfd, RX_buf, sizeof(RX_buf));	//返回读取的字节数
	if (n <= 0) {
		return -1;
	}
	std::copy(RX_buf, RX_buf + n, mBuffer);	//复制前n个元素
	return n;
}

int Buffer::write(int sockfd) {
	int ret = ::write(sockfd, (char*)mBuffer, mBufferSize);
	retriveveAll();
	return ret;
}

void Buffer::retriveveAll() {
	memset(mBuffer, 0, RX_SIZE);
	mBufferSize = 0;
}