/********************************************************************************
 *      Copyright:  (C) 2024 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  socket_client.c
 *    Description:  This file 
 *
 *        Version:  1.0.0(24/12/23)
 *         Author:  Qinyuan <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "24/12/23 15:31:52"
 *                 
 ********************************************************************************/
#include "socket_client.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MSG_STR "Hello Qinyuan\n"


/**
 * @brief 与服务器建立连接。命令行传参
 * @param servip 服务器的IP地址
 * @param port  服务器端口
 * @return int  返回套接字描述符，如果失败返回负值
 */
int create_conn(const char *servip, int port)
{
    int                             sockfd = -1;
    struct sockaddr_in              servaddr; // IPv4

    // 1.创建socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Create socket failure: %s\n", strerror(errno));
        return -1;
    }
    printf("Create socket[%d] successfully!\n", sockfd);

    // 2.填写服务器信息
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, servip, &servaddr.sin_addr);

    // 3.连接服务器
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("Connect to server [%s:%d] failure: %s\n", servip, port, strerror(errno));
        return -2;
    }

    printf("Connect to server [%s:%d] successfully!", servip, port);
    return sockfd;
}


/**
 * @brief 向服务器发送消息
 * @param sockfd 套接字描述符
 * @param message 发送的消息内容
 * @return int 返回0表示成功，负值表示失败
 */
int send_msg(int sockfd, const char *data, size_t data_size)
{
    ssize_t rv = write(sockfd, data, data_size); // ssize_t是有符号的整数类型，大小与平台相关
    if (rv < 0)
    {
        printf("Write to server failure: %s\n", strerror(errno));
    }
    return rv;
}


/**
 * @brief 从服务器接收数据
 * @param sockfd 套接字描述符
 * @param buf 接受消息的缓冲区
 * @param buf_size 缓冲区大小
 * @return 成功返回接收到的字节数，失败返回负值，服务器断开连接返回0
  */ 
int receive_msg(int sockfd, char *buf, size_t buf_size)
{
    ssize_t rv = read(sockfd, buf, buf_size);
    if (rv < 0)
    {
        printf("Read data from server by socket[%d] failure: %s\n",sockfd, strerror(errno));
        return -1;
    }
    else if (rv == 0)
    {
        printf("socket[%d] get disconnected\n", sockfd);
        return 0; // 服务器断开连接
    }
    else
    {
        printf("Read %d bytes data from server: %s\n", rv, buf);
    }
    return rv;
}


/**
 * @brief 关闭连接
 * @param sockfd 套接字描述符
*/ 
void close_conn(int sockfd)
{
    close(sockfd);
    printf("Connection closed.\n");
}