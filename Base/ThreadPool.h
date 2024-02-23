#ifndef TRLIB_THREADPOOL_H
#define TRLIB_THREADPOOL_H
#include<queue>
#include<vector>
#include"Thread.h"
#include<mutex>
#include<condition_variable>

class Task {
public:
	typedef void (*TaskCallback)(void*);	//任务回调函数，函数指针
	static Task* createNew();	
	Task();
	void setTaskCallback(TaskCallback cb, void* arg);
	void handle();

private:
	TaskCallback mTaskCallback;	//任务回调函数
	void* mArg;	//回调函数参数
};

class ThreadPool {
public:
	static ThreadPool* createNew(int num);
	explicit ThreadPool(int num);	//explicit关键字，禁止隐式转换
	~ThreadPool();

	void addTask(Task* task, std::string name);

private:
	void loop();
	void createThreads();	//创建线程
	void cancelThreads();	//取消线程
	class MThread : public Thread {
		void run(void* arg);
	};

private:
	std::queue<Task*>mTaskQue;	//任务队列，每个任务是一个Task对象，共享资源
	std::mutex mMtx;	//互斥锁，保证每次只有一个线程能访问任务队列
	std::condition_variable mCon;	//条件变量，用于线程同步
	std::vector<MThread> mThreads;	//线程数组，共享资源
	bool mQuit;	//线程池退出标志，共享资源
};
/*当任务队列为空时，工作线程会等待在条件变量mCon上；
当有新的任务添加到任务队列时，条件变量mCon会被唤醒，工作线程会继续执行。
这个过程需要互斥量mMtx的保护，以防止竞态条件。
访问共享资源都需加锁*/

#endif // !TRLIB_THREADPOOL_H
