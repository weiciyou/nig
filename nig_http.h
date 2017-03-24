/*************************************************************************
    * File: 1-1.c
    * Brief:
    * Author: luoyuxiang
    * Mail: 269724033@qq.com
    * Created Time: Sat 17 Dec 2016 08:00:39 PM CST
 ************************************************************************/

#ifndef _NIG_HTTP_H_INCLUDED_
#define _NIG_HTTP_H_INCLUDED_

#include "nig_header.h"
#include "nig_config.h"

/*
 * 请求头部结构体
 * 只存储请求行和类型、长度字段，其他信息忽略
 * 也存储了从请求地址中分析出的请求文件名和查询参数
 */
typedef struct {
    char uri[256];          // 请求地址
    char method[16];        // 请求方法
    char version[16];       // 协议版本
    char filename[256];     // 请求文件名(包含完整路径)
    char name[256];         // 请求文件名(不包含路径，只有文件名)
    char cgiargs[256];      // 查询参数
    char contype[256];      // 请求体类型
    char conlength[16];     // 请求体长度
    char *args_post;
	int fd;                 //文件描述符
	int epollfd;            //epoll 描述符
	char remote_ip[16];     //客户端ip
    int remote_port;        //客户端端口
} http_ret;

//处理fastcgi   从描述符fd中读取n个字节到存储器位置usrbuf
ssize_t http_readn(int fd, void *usrbuf, size_t n);

//处理fastcgi   将usrbuf缓冲区中的前n个字节数据写入fd中
ssize_t http_writen(int fd, void *usrbuf, size_t n);


//获取一行
ssize_t http_get_line_t(int sock, char *buf, int size);

//获取请求头
void http_get_headers(int fd, http_ret *ret); // 读取请求行

//解析uri
int http_parse_uri(char *uri, char *filename, char *name, char *cgiargs);

//发送头
void http_send_headers(int client);

//查看一个文件
void http_cat(int, FILE *);

//500错误
void http_unimplemented(int);

//404错误
void http_not_found(int);

void http_error_die(const char *sc);
#endif    /*end ifndef*/
