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
	mThread = std::thread(&threadRun, this);
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
	mThread.detach();
	mIsDetach = true;
	return true;
}

bool Thread::join() {
	if (!mIsStart || mIsDetach) {
		return false;
	}
	mThread.join();
	return true;
}

void* Thread::threadRun(void* arg) {
	Thread* thread = (Thread*)arg;
	thread->run(thread->mArg);
	return NULL;
}
