/*************************************************************************
    * File: 1-1.c
    * Brief:
    * Author: luoyuxiang
    * Mail: 269724033@qq.com
    * Created Time: Sat 17 Dec 2016 08:00:39 PM CST
 ************************************************************************/

#ifndef _NIG_EPOLL_H_INCLUDED_
#define _NIG_EPOLL_H_INCLUDED_

#include "nig_header.h"
#include "nig_config.h"
#include "nig_http.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

struct epoll_event *evlist;

NIG_API int epoll_setnonblocking(int fd);

// 创建服务端  套接字并进行绑定
NIG_API  int epoll_socket_servers(const char* ip,int port);

/*
 * 创建连接fastcgi服务器的客户端套接字
 * 出错返回-1
 */
NIG_API  int epoll_socket_client(const char* fastcgi_host,int fastcgi_port);

//创建epoll
NIG_API  int epoll_create_event(int flag);

//添加事件
NIG_API void epoll_add_event(http_ret * ret, int state);

//修改事件
NIG_API  void epoll_update_event(http_ret * ret, int state);

//删除事件
NIG_API  void epoll_delete_event(http_ret * ret, int state);

#endif    /*end ifndef*/
