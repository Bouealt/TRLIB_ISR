#ifndef TRLIB_EVENT_H
#define TRLIB_EVENT_H
#include <iostream>
typedef void (*EventCallback)(void *, int);
typedef void (*EventSendCallback)(void *, int fd, std::string);

class TriggerEvent
{
public:
    static TriggerEvent *createNew(void *arg, int fd, std::string mess = "");
    static TriggerEvent *createNew();

    TriggerEvent(void *arg, int fd, std::string mess);
    ~TriggerEvent();

    void setArg(void *arg) { mArg = arg; }
    void setTriggerCallback(EventCallback cb) { mTriggerCallback = cb; }
    void setSendCallback(EventSendCallback cb) { mSendCallback = cb; }
    void handleEvent();

public:
    int mFd;
    void *mArg;
    std::string mMess;
    EventCallback mTriggerCallback = NULL;
    EventSendCallback mSendCallback = NULL;
};

class TimerEvent
{
public:
    static TimerEvent *createNew(void *arg, int fd, std::string name);

    TimerEvent(void *arg, int fd, std::string name);
    ~TimerEvent();

    void setArg(void *arg) { mArg = arg; }
    void setTimeoutCallback(EventCallback cb) { mTimeoutCallback = cb; }
    bool handleEvent();
    void stop();
    void start();
    bool isStop();
    std::string getName();

private:
    int mFd;
    void *mArg;
    EventCallback mTimeoutCallback;
    bool mIsStop;
    std::string mName;
};

class IOEvent
{
public:
    enum IOEventType
    {
        EVENT_NONE = 0,
        EVENT_READ = 1 << 0,  // 1
        EVENT_WRITE = 1 << 1, // 2
        EVENT_ERROR = 1 << 2, // 4
    };

    static IOEvent *createNew(int fd, void *arg);
    static IOEvent *createNew(int fd);

    IOEvent(int fd, void *arg);
    ~IOEvent();

    int getFd() const { return mFd; }
    int getEvent() const { return mEvent; }
    void setEvent(int event) { mEvent = event; }
    void setArg(void *arg) { mArg = arg; }

    void setReadCallback(EventCallback cb) { mReadCallback = cb; };
    void setWriteCallback(EventCallback cb) { mWriteCallback = cb; };
    void setErrorCallback(EventCallback cb) { mErrorCallback = cb; };

    void enableReadHandling() { mEvent |= EVENT_READ; }
    void enableWriteHandling() { mEvent |= EVENT_WRITE; }
    void enableErrorHandling() { mEvent |= EVENT_ERROR; }
    void disableReadeHandling() { mEvent &= ~EVENT_READ; }
    void disableWriteHandling() { mEvent &= ~EVENT_WRITE; }
    void disableErrorHandling() { mEvent &= ~EVENT_ERROR; }

    bool isNoneHandling() const { return mEvent == EVENT_NONE; }
    bool isReadHandling() const { return (mEvent & EVENT_READ) != 0; }
    bool isWriteHandling() const { return (mEvent & EVENT_WRITE) != 0; }
    bool isErrorHandling() const { return (mEvent & EVENT_ERROR) != 0; } ////-----

    void handleEvent();

private:
    int mFd;
    void *mArg;
    int mEvent = EVENT_NONE;
    EventCallback mReadCallback = NULL;
    EventCallback mWriteCallback = NULL;
    EventCallback mErrorCallback = NULL;
};

#endif