#ifndef TRLIB_BUFFER_H
#define TRLIB_BUFFER_H
#include <stdint.h>
class Buffer {
public:
	Buffer();
	~Buffer();

	int read(int sockfd);
	int write(int sockfd);
	void retriveveAll();

	char* mBuffer;
	int mBufferSize;
};
#endif // !TRLIB_BUFFER_H

