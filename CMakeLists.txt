cmake_minimum_required(VERSION 3.26)
project(TRLIB_ISR_Windows)

set(CMAKE_CXX_STANDARD 14)

include_directories(Base)
include_directories(Driver)
include_directories(TRLIB_ISR)

add_executable(TRLIB_ISR_Windows
        Base/EpollPoller.cpp
        Base/EpollPoller.h
        Base/Event.cpp
        Base/Event.h
        Base/EventScheduler.cpp
        Base/EventScheduler.h
        Base/Log.h
        Base/Poller.cpp
        Base/Poller.h
        Base/SelectPoller.cpp
        Base/SelectPoller.h
        Base/SocketsOps.cpp
        Base/SocketsOps.h
        Base/Thread.cpp
        Base/Thread.h
        Base/ThreadPool.cpp
        Base/ThreadPool.h
        Base/Timer.cpp
        Base/Timer.h
        Driver/deCell4g.cpp
        Driver/deCell4g.h
        Driver/usbctl.cpp
        Driver/usbctl.h
        TRLIB_ISR/Buffer.cpp
        TRLIB_ISR/Buffer.h
        TRLIB_ISR/Device.cpp
        TRLIB_ISR/Device.h
        TRLIB_ISR/InetAddress.cpp
        TRLIB_ISR/InetAddress.h
        TRLIB_ISR/ISRConnection.cpp
        TRLIB_ISR/ISRConnection.h
        TRLIB_ISR/ISRServer.cpp
        TRLIB_ISR/ISRServer.h
        TRLIB_ISR/main.cpp
        TRLIB_ISR/TcpConnection.cpp
        TRLIB_ISR/TcpConnection.h
        TRLIB_ISR/TRServer.cpp
        TRLIB_ISR/TRServer.h)
