#ifndef BXC_RTSPSERVER_LOG_H
#define BXC_RTSPSERVER_LOG_H
#include <time.h>
#include <string>
#include <vector>

static std::string getTime() {
    const char* time_fmt = "%Y-%m-%d %H:%M:%S";
    time_t t = time(nullptr);
    char time_str[64];
    strftime(time_str, sizeof(time_str), time_fmt, localtime(&t));

    return time_str;
}

#define LOGI(format, ...)  fprintf(stderr,"[INFO]%s [%s:%d] " format "\n", getTime().data(),__FILE__,__LINE__,##__VA_ARGS__)
#define LOGE(format, ...)  fprintf(stderr,"[ERROR]%s [%s:%d] " format "\n",getTime().data(),__FILE__,__LINE__,##__VA_ARGS__)
#endif //BXC_RTSPSERVER_LOG_H