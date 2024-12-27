#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <ctype.h>
#include <time.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>

static  inline void msleep(unsigned long ms); // 用 select 实现以毫秒为单位的睡眠
int     socket_server_init(char *listen_ip, int listen_port); // 启动函数，包含socket(),bind()和listen()

#define ARRAY_SIZE(x)   (sizeof(x)/sizeof(x[0]))
#define MAX_SN_LEN 32

typedef struct
{
    time_t  timestamp;    // 当前时间
    char    serial_no[MAX_SN_LEN]; // 序列号
    float   temperature; //当前温度
}data_t;

#endif SOCKET_SERVER_H