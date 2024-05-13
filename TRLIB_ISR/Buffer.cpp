#include"Buffer.h"
#include "../Base/SocketsOps.h"
#include<string.h>
#include <stdlib.h>
#include <algorithm>


Buffer::Buffer()
{
	mBufferSize = 0;
	mBuffer = (char*)malloc(RX_SIZE);
	memset(mBuffer, 0, RX_SIZE);
}

Buffer::~Buffer() {
	free(mBuffer);
}

int Buffer::read(int sockfd) {
	char RX_buf[RX_SIZE];
	memset(RX_buf, 0, sizeof(RX_buf));
	int n = ::read(sockfd, RX_buf, sizeof(RX_buf));
	if (n <= 0) {
		return -1;
	}
	std::copy(RX_buf, RX_buf + n, mBuffer);
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