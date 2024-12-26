/********************************************************************************
 *      Copyright:  (C) 2024 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  socket_client.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(24/12/23)
 *         Author:  Qinyuan <iot25@lingyun>
 *      ChangeLog:  1, Release initial version on "24/12/23 15:31:52"
 *                 
 ********************************************************************************/

#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

int     create_conn(const char *servip, int port);
int     send_msg(int sockfd, const char *data, size_t data_size);
int     receive_msg(int sockfd, char *buf, size_t buf_size);
void    close_conn(int sockfd);
void    print_usage(char *progname);

#endif // SOCKET_CLIENT_H