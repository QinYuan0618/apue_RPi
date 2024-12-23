/********************************************************************************
 *      Copyright:  (C) 2024 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  ds18b20.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(20/12/24)
 *         Author:  LingYun <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "20/12/24 15:31:52"
 *                 
 ********************************************************************************/
// ds18b20.h
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

int get_temperature(float *temp);

#endif // DS18B20_H 