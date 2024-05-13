#ifndef TRLIB_THREADPOOL_H
#define TRLIB_THREADPOOL_H
#include<queue>
#include<vector>
#include"Thread.h"
#include<mutex>
#include<condition_variable>

class Task {
public:
	typedef void (*TaskCallback)(void*);
	static Task* createNew();
	Task();
	void setTaskCallback(TaskCallback cb, void* arg);
	void handle();

private:
	TaskCallback mTaskCallback;
	void* mArg;
};

class ThreadPool {
public:
	static ThreadPool* createNew(int num);
	explicit ThreadPool(int num);
	~ThreadPool();

	void addTask(Task* task, std::string name);

private:
	void loop();
	void createThreads();
	void cancelThreads();
	class MThread : public Thread {
		void run(void* arg);
	};

private:
	std::queue<Task*>mTaskQue;
	std::mutex mMtx;
	std::condition_variable mCon;
	std::vector<MThread> mThreads;
	bool mQuit;
};


#endif // !TRLIB_THREADPOOL_H
