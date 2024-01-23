#ifndef TRLIB_USBCTL_H
#define TRLIB_USBCTL_H

#include<stdint.h>
#include<string>

namespace usbctl {
    int openPort(int comport);
    int setOpt(int fd, int nSpeed, int nBits, uint8_t nEvent, int nStop);
    std::string getTime();
}




#endif // !TRLIB_USBCTL_H
