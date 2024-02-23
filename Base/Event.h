#ifndef TRLIB_EVENT_H
#define TRLIB_EVENT_H
#include<iostream>
typedef void (*EventCallback)(void*, int);  //函数指针，用于回调函数
typedef void (*EventSendCallback)(void*, int fd, std::string);

//触发事件
class TriggerEvent {
public:
    static TriggerEvent* createNew(void* arg, int fd, std::string mess = "");   //静态工厂模式
    static TriggerEvent* createNew();   //静态工厂模式，无参数

    TriggerEvent(void* arg, int fd, std::string mess);
    ~TriggerEvent();

    void setArg(void* arg) { mArg = arg; }
    void setTriggerCallback(EventCallback cb) { mTriggerCallback = cb; }    //设置回调函数
    void setSendCallback(EventSendCallback cb) { mSendCallback = cb; }  //设置回调函数
    void handleEvent(); //处理事件

public:
    int mFd;    //触发事件的fd
    void* mArg; //回调函数参数，void*类型，可以指向任意类型
    std::string mMess;  
    EventCallback mTriggerCallback = NULL;  //触发回调函数
    EventSendCallback mSendCallback = NULL; //发送回调函数
};

//定时器事件
class TimerEvent {
public:
    static TimerEvent* createNew(void* arg, int fd);    //静态工厂模式
    static TimerEvent* createNew(); //静态工厂模式，无参数

    TimerEvent(void* arg, int fd);
    ~TimerEvent();

    void setArg(void* arg) { mArg = arg; }
    void setTimeoutCallback(EventCallback cb) { mTimeoutCallback = cb; }    //设置超时回调函数
    bool handleEvent();
    void stop();
    void start();
    bool isStop();
    void addExeTimes(); //执行次数加1
    void setTimerId(uint32_t timerId);  //设置定时器id
    uint32_t getTimerId();

private:
    int mFd;    //定时器事件的fd
    void* mArg; //函数参数，void*类型，可以指向任意类型
    EventCallback mTimeoutCallback; //超时回调函数
    bool mIsStop;   //定时器停止标志
    int mExeTimes;  //执行次数
    uint32_t mTimerId;  //定时器id
};

//IO事件
class IOEvent {
public:
    enum IOEventType
    {
        EVENT_NONE = 0,     //0000
        EVENT_READ = 1,     //0001
        EVENT_WRITE = 2,    //0010
        EVENT_ERROR = 4,    //0100
    };

    static IOEvent* createNew(int fd, void* arg);

    IOEvent(int fd, void* arg);
    ~IOEvent();

    int getFd() const { return mFd; }
    int getEvent() const { re turn mEvent; }
    void setEvent(int event) { mEvent = event; }
    void setArg(void* arg) { mArg = arg; }

    void setReadCallback(EventCallback cb) { mReadCallback = cb; };
    void setWriteCallback(EventCallback cb) { mWriteCallback = cb; };
    void setErrorCallback(EventCallback cb) { mErrorCallback = cb; };

    void enableReadHandling() { mEvent |= EVENT_READ; } //按位或
    void enableWriteHandling() { mEvent |= EVENT_WRITE; }
    void enableErrorHandling() { mEvent |= EVENT_ERROR; }
    void disableReadeHandling() { mEvent &= ~EVENT_READ; }  //按位与
    void disableWriteHandling() { mEvent &= ~EVENT_WRITE; }
    void disableErrorHandling() { mEvent &= ~EVENT_ERROR; }

    bool isNoneHandling() const { return mEvent == EVENT_NONE; }
    bool isReadHandling() const { return (mEvent & EVENT_READ) != 0; }
    bool isWriteHandling() const { return (mEvent & EVENT_WRITE) != 0; }
    bool isErrorHandling() const { return (mEvent & EVENT_ERROR) != 0; }    //按位与

    void handleEvent();

private:
    int mFd;    //IO事件的fd
    void* mArg; //参数
    int mEvent = EVENT_NONE;    //事件类型
    EventCallback mReadCallback = NULL;     //读回调函数
    EventCallback mWriteCallback = NULL;    //写回调函数
    EventCallback mErrorCallback = NULL;    //错误回调函数
};


#endif