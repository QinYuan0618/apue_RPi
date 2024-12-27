/********************************************************************************
 *      Copyright:  (C) 2024 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  socket_server.c
 *    Description:  This file 
 *
 *        Version:  1.0.0(24/12/23)
 *         Author:  Qinyuan <qyts1989@163.com>
 *      ChangeLog:  1, Release initial version on "24/12/23 15:31:52"
 *                 
 ********************************************************************************/
#include "socket_server.h"
#include "tem_database.c"


int main(int argc, char **argv)
{
    int             serv_port = 0;   // 客户端指定的服务器用于监听的端口
    int             daemon_run = 0;  // 是否让程序当后台去运行, -b
    char           *progname = NULL;     
    int             opt;
    int             listenfd, connfd;
    int             rv;
    int             i, j;
    int             found;           // 作为当前监听表 fds_array 是否还有空位的 flag
    int             maxfd = 0;       // 当前 fds_array 数组中 有效文件描述符 的最大值, 是 select 函数的参数之一
    data_t          data;            // 读缓冲区，结构体：时间(rawtime格式), 序列号，温度
    fd_set          rdset;           // 读操作的描述符集合
    int             fds_array[1024]; // 监听的文件描述符 fd 数组, 程序通过这些文件描述符来监视多个文件或者套接字的状态变化
    struct option   long_options[] =
    {   //长选项名, 选项需要参数与否, 选项值的存储地址, 该选项对应的短选项字符 -p
        {"daemon", no_argument, NULL, 'b'},
        {"port", required_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    progname = basename(argv[0]);
    while ((opt = getopt_long(argc, argv, "bp:h", long_options, NULL)) != -1)
    {
        //getopt_long 函数的返回值是 int 类型, 看似字符实则是 ASCII 码在比较
        switch (opt)
        {
        case 'b': //case 标签要求一个整数常量或字符常量, 不能使用双引号"
            daemon_run = 1;
            break;

        case 'p':
            serv_port = atoi(optarg);
            break;

        case 'h':
            print_usage(progname);
            break;

        default: //当 switch 表达式的值与任何 case 标签都不匹配时，witch 会直接跳到 default 分支执行
            break;
        }
    }
    
    /* 如果没有设置端口值 serv_port 就打印错误信息*/
    if (!serv_port)
    {
        print_usage(progname); //print_usage 用于显示程序的用法信息, 通常包括选项/参数以及示例
        return -1;
    }

    /* 1.初始化服务器监听套接字listenfd,这包括socket(),bind()以及listen() */
    if ( (listenfd=socket_server_init(NULL,serv_port)) < 0 )
    {
        printf("Server failed to listen on port %d.\n", serv_port);
        return -2;
    }
    printf("Server start to listen on port %d.\n", serv_port);

    /* 是否让当前程序到后台去运行 */
    if (daemon_run)
    {
        daemon(0, 0);
    }

    /* 初始化，清空监听表fds_array */
    for (i = 0; i < ARRAY_SIZE(fds_array); i++) // ARRAY_SIZE 是求一个数组里有多少个元素, 宏定义
    {
        fds_array[i] = -1; // -1 代表这个位置为空
    }
    fds_array[0] = listenfd; //将第一个要监听的描述符加入描述符数组fds_array

    /* 一趟是一次select */
    for (; ; )
    {
        FD_ZERO(&rdset);

        /* 遍历监听数组 fds_array, 将非空的 fd 加入 rdset 中*/
        for (i = 0; i < ARRAY_SIZE(fds_array); i++)
        {
            if (fds_array[i] < 0)
                continue; 

            maxfd = fds_array[i]>maxfd ? fds_array[i] : maxfd;
            FD_SET(fds_array[i], &rdset); //把文件描述符 fds_array[i] 加入可读表 rdset 里去
        }

        /* 2.select开始监听, program will blocked here */
        rv = select(maxfd + 1, &rdset, NULL, NULL, NULL); // 只关心读, 且永不超时
        if (rv < 0) // 读失败, 终止for循环
        {
            printf("select failure: %s.\n", strerror(errno));
            break;
        }
        if (rv == 0) // 读超时, 终止本次循环, 开始下一趟
        {
            printf("select get timeout.\n");
            continue;
        }
        else // 读成功，有新事件发生
        {
            /*
                rv > 0 : 检测到有一个或者多个文件描述符已经准备好进行 I/O 操作(如：读操作)
                用 FD_ISSET() 检查具体哪个文件描述符准备好进行操作
                (1). listenfd 新客户端连接请求
                (2). 已连接的客户端有数据到来
            */
            
            /* 3.1 情况(1):新客户端连接请求 */
            if (FD_ISSET(listenfd, &rdset))
            {
                /* 接受客户端请求 accept() */
                if ((connfd = accept(listenfd, (struct  sockaddr *)NULL, NULL)) < 0) // 不关心客户端的 IP 地址和端口号
                {
                    printf("accept new client failure: %s.\n", strerror(errno));
                    continue;
                }

                found = 0; // 作为 fds_array 目前是否还有空位的flag
                /* 在 监听列表 fds_array 中顺序找第一个空位并把 conn_fd 放入 */
                for (i = 0; i < ARRAY_SIZE(fds_array); i++)
                {
                    if (fds_array[i] < 0)
                    {
                        printf("accept new client[%d] and add it into array.\n", connfd);
                        fds_array[i] = connfd;
                        found =  1;
                        break;
                    }
                }  
                if (!found) //fds_array 没有空位了
                {
                    printf("accept new client[%d] but full, so refuse it.\n", connfd);
                    close(connfd);
                }  
            }

            /* 情况(2):已连接的客户端有数据到来 */
            else
            {
                /* 遍历 fds_array 找到准备好的读操作描述符 */
                for (i = 0; i < ARRAY_SIZE(fds_array); i++)
                {   
                    if (fds_array[i]<0 || !FD_ISSET(fds_array[i], &rdset)) // 如果是空, 或者不是发生读事件, 跳过
                        continue;
                    
                    /* 读read() */
                    if ( (rv=read(fds_array[i], &data, sizeof(data))) <= 0 )
                    {
                        printf("socket[%d] read failure or get disconnect.\n", fds_array[i]);
                        close(fds_array[i]);
                        fds_array[i] = -1;
                    }
                    /* 读成功 */
                    else
                    {
                        printf("socket[%d] read get %d bytes data.\n", fds_array[i], rv);

                        /* 存入 sqlite 数据库 */
                        open_db();
                        insert_data(data.timestamp, data.serial_no, data.temperature);
                        show_data();

                        /* 写回, 如果写出错也要关掉 */
                        if (write(fds_array[i], &data, rv) < 0)
                        {
                            printf("socket[%d] write failure: %s.", fds_array[i], strerror(errno));
                            close(fds_array[i]);
                            fds_array[i] = -1;
                        }
                    }
                }
            }
        }

            
    } 
CleanUp:
    close(listenfd);
    return 0;
}


/**
 * @brief 这是一个封装了socket()、bind()、listen()的启动函数
 * @param listen_ip 要监听的客户端ip，可以为空
 * @param listen_port 监听的端口，不能为空，与客户端指定的相匹配
 * @return rv，成功返回正值，失败返回负值 
 */
int socket_server_init(char *listen_ip, int listen_port)
{
    struct sockaddr_in  servaddr;
    int                 rv = 0;
    int                 on = 1;   // 指向设置值的指针
    int                 listenfd;

    /* 1.创建socket */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Use socket() to create a TCP socket failure: %s\n", strerror(errno));
        return -1;
    }
    
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)); // SO_REUSEADDR 可以让服务器在端口仍处于 TIME_WAIT 状态时重新绑定该端口，避免了端口冲突或无法立即使用的问题

    /* 绑定服务器端口*/
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(listen_port);
    if (!listen_ip) // 如果没有指定要监听的ip
    {
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有ip
    }
    else // 监听指定的ip
    {
        if (inet_pton(AF_INET, listen_ip, &servaddr.sin_addr) <= 0)
        {
            printf("inet_pton() set listen IP address failure.\n");
            rv = -2;
            goto CleanUp;
        }
    }
    
    /* 2.bind() 将 fd 与端口绑定*/
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("Use bind() to bind the TCP socket failure: %s\n",strerror(errno));
        rv = -3;
        goto CleanUp;
    }

    /* 3.监听客户端发来的连接请求 */
    if (listen(listenfd, 13) < 0)
    {
        printf("Use bind() to bind the TCP socket failure: %s.\n", strerror(errno));
        rv = -4;
        goto CleanUp;
    }
  
CleanUp:
    if (rv<0)
        close(listenfd);
    else
        rv = listenfd;
    
    return rv;
}


/**
 * @brief 命令行传参提示信息
 * @param progname 文件名称
 */
void print_usage(char *progname)
{
    printf("%s usage: \n", progname);
    printf(" %s is a socket server program, which used to verify client and echo back string from it.\n", progname);

    printf("-b(--daemon):set program runnning on background.\n");
    printf("-p(--port):specify socket server port address.\n");
    printf("-h(--help):display this help information.\n");
    printf("\nExample: %s -b -p 8900\n", progname);
}