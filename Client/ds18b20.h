/********************************************************************************
 *      Copyright:  (C) 2024 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  ds18b20.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(24/12/23)
 *         Author:  Qinyuan <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "24/12/23 15:31:52"
 *                 
 ********************************************************************************/

#ifndef DS18B20_H // 如果没有定义 DS18B20_H
#define DS18B30_H // 定义 DS18B20_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>                     
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_SN_LEN 32

typedef struct
{
    time_t  timestamp;    // 当前时间
    char    serial_no[MAX_SN_LEN]; // 序列号
    float   temperature; //当前温度
}data_t;

data_t get_temperature();

#endif // DS18B20_H 