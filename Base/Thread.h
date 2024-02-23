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
	virtual void run(void* arg) = 0;	//纯虚函数，在子类中重写

private:
	static void* threadRun(void*);

private:
	void* mArg;	//线程参数
	bool mIsStart;	//线程是否启动
	bool mIsDetach;	//线程是否分离
	std::thread mThread;	//线程对象
};


#endif