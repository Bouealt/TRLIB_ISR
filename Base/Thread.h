#ifndef TRLIB_THREAD_H
#define TRLIB_THREAD_H
#include<thread>

class Thread {
public:
	virtual ~Thread();

	bool start(void* arg);
	bool detach();
	bool join();

protected:
	Thread();
	virtual void run(void* arg) = 0;

private:
	static void* threadRun(void*);

private:
	void* mArg;
	bool mIsStart;
	bool mIsDetach;
	std::thread mThread;
};


#endif