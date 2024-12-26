#include "ds18b20.h"
#include "socket_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

//studio.weike-iot.com 2216 "192.168.0.26" 12345


/** 
 * @brief 主程序：客户端与服务器建立连接，获取温度并定时上传。服务器的 ip 和 端口号 需要命令行传参获取
*/
int main(int argc, char **argv)
{
    data_t                  data; // ds18b20.c 获取到的数据结构体，其中包括时间、序列号、温度值
    int                     sockfd = -1;
    char                    buf[1024];
    ssize_t                 rv;
    int                     port = 0;
    char                   *servip = NULL;
    struct option           opts[] = {
        {"ipaddr", required_argument, NULL, 'i'},
        {"port", required_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0} // 结束标志
    };
    int                             ch; // switch变量

    /* 1.命令行传参获取服务器的 ip 和 端口号 */
    while ((ch = getopt_long(argc, argv, "i:p:h", opts, NULL)) != -1)
    {
        switch (ch)
        {
        case 'i':
            servip = optarg; // 将服务器 IP 地址赋给 servip: 192.168.xx.xx

        case 'p':
            port = atoi(optarg); // 端口号赋给 port
            break;

        case 'h':
            print_usage(argv[0]); // 显示用法说明
            return 0;
        }
    }
    if (port <= 0 || !servip)
    {
        print_usage(argv[0]);
        return 0;
    }

    /* 2.创建socket连接到服务器 */
    sockfd = create_conn(servip, port);
    if (sockfd < 0) {
        printf("Failed to create connection\n");
        return -1;
    }

    while (1)
    {
        /* 3.获取温度并上报 */
        data = get_temperature();
        if (data.temperature != 0.0)
        {
            /* 4.数据上报给服务器*/
            rv = send_msg(sockfd, &data, sizeof(data));
            if (rv < 0)
            {
                printf("Failed to send data to server\n");
                close_conn(sockfd); // 发送失败，关闭连接
                return -2;
            }
            /* 5.服务器响应 */
            rv = receive_msg(sockfd, buf, sizeof(buf));
            if (rv < 0)
            {
                printf("Failed to receive data from server\n");
                close_conn(sockfd); // 接收失败，关闭连接
                return -3;
            }
        }
        else
        {
            printf("Failed to get temperature\n");
        }

        // 每30分钟读取一次温度并发送,这里设置30秒，用于测试
        sleep(30); // 每半分钟运行一次
    }

    /* 5.关闭连接 */
    close_conn(sockfd);
    
    return 0;
}

/**
 * @brief 命令行传参提示信息
 * @param progname 文件名称
 */
void print_usage(char *progname)
{
    printf("%s usage: \n", progname);
    printf("-p(--server_port):specify server port.\n");
    printf("-i(--server_ip):specify server ip.\n");
    printf("-h(--help):print this help infomation.\n");
}