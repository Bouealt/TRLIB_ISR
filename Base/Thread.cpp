#include"Thread.h"

Thread::Thread() :
	mArg(NULL),
	mIsStart(false),
	mIsDetach(false)
{

}

Thread::~Thread() {
	if (mIsStart && !mIsDetach) {
		detach();
	}
}

bool Thread::start(void* arg) {
	mArg = arg;
	mThread = std::thread(&threadRun, this);	//创建新线程，执行threadRun函数，参数是this，指向当前对象
	mIsStart = true;
	return true;
}

bool Thread::detach() {
	if (!mIsStart) {
		return false;
	}
	if (mIsDetach) {
		return true;
	}
	mThread.detach();	//分离线程，std::thread类中的detach函数
	mIsDetach = true;
	return true;
}

bool Thread::join() {
	if (!mIsStart || mIsDetach) {
		return false;
	}
	mThread.join();	//等待线程结束，std::thread类中的join函数
	return true;
}

void* Thread::threadRun(void* arg) {	//arg参数是指向Thread对象的指针
	Thread* thread = (Thread*)arg;
	thread->run(thread->mArg);	//执行run函数，run为纯虚，在子类中重写功能
	return NULL;
}
