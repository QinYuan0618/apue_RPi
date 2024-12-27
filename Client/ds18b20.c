/*********************************************************************************
 *      Copyright:  (C) 2024 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  ds18b20.c
 *    Description:  This file is detect ds18b20's tem.
 *                 
 *        Version:  1.0.0(20/12/24)
 *         Author:  Qinyuan <qyts1989@163.com>
 *      ChangeLog:  1, Release initial version on "24/12/24 15:29:54"
 *                 
 ********************************************************************************/

#include "ds18b20.h"

 /*
 * @brief 获取温度功能函数,并打印获取时间，序列号与温度
 * @param temp 存储获取温度数据的指针
 * @return int 返回0表示成功，否则错误
 */
data_t get_temperature()
{
  data_t                  data = {0};     // 初始化结构体
  int                     fd = -1;        // 文件描述符
  char                    buf[256];       // 存储读取数据的缓冲区
  char                   *ptr = NULL;     // 指向数据中的 "t=" 字符串位置
  DIR                    *dirp = NULL;    // 目录指针
  struct  dirent         *direntp = NULL; // 目录项
  char                    w1_path[64] = "/sys/bus/w1/devices/"; // w1设备路径
  char                    chip_sn[32];    // DS18B20 芯片的序列号
  int                     found = 0;      // 是否找到设备的标志
  time_t                  rawtime;        // time 函数的返回值
  struct  tm             *timeinfo;       // 存储时间的各个部分：年月日时分秒
  char                    time_buffer[80];

  /* 获取当前时间 */
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo); //格式化时间
  data.timestamp = rawtime; // 存储timestamp 更利于存储和传输

  /* 1.打开w1设备目录, dirp指向这个目录 */
  dirp = opendir(w1_path);
  if (!dirp)
  {
    printf("open folder %s failure: %s\n", w1_path, strerror(errno));
    return data;
  }

  /* 2.查找以"28-"开头的设备名称，表示DS18B20，将设备名称赋值给chip_sn */
  while (NULL != (direntp = readdir(dirp)))
  {
    if (strstr(direntp->d_name, "28-"))
    {
      strncpy(chip_sn, direntp->d_name, sizeof(chip_sn));
      found = 1;
    }
  }

  closedir(dirp); //关闭目录

  // 如果没找到该设备，返回错误
  if (!found)
  {
    printf("can not found ds18b20 chipset\n"); 
    return data;
  }

  /* 3.构建设备路径 /sys/bus/w1/devices/28-xxxxxxx/w1_slave */
  strncat(w1_path, chip_sn, sizeof(w1_path) - strlen(w1_path));
  strncat(w1_path, "/w1_slave", sizeof(w1_path) - strlen(w1_path));

  /* 4.根据设备路径打开设备文件 */
  if ( (fd=open(w1_path, O_RDONLY)) < 0 )
  {
    // printf("open file failure: %s\n", strerror(errno));
    perror("open file failure");
    return data;
  }

  /* 5.读取数据 */
  memset(buf, 0, sizeof(buf));
  if (read(fd, buf, sizeof(buf)) < 0)
  {
    printf("read data from fd=%d failure: %s\n", fd, strerror(errno));
    return data;
  }

  /* 6.找到"t="字符串，获取温度值。原为...t=11625 */
  ptr = strstr(buf, "t=");
  if (!ptr)
  {
    printf("can not find t= string\n");
    return data;
  }

  ptr += 2; // 跳过"t="

  /* 7.温度数据转为浮点数并储存 */
  data.temperature = atof(ptr) / 1000;
  strncpy(data.serial_no, chip_sn, sizeof(data.serial_no));
  printf("[current_time]: %s\n[chip_sn     ]: %s\n[Temperature ]: %f℃\n", time_buffer, data.serial_no, data.temperature);

  close(fd);

  return data;
}