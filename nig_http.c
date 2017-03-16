/*************************************************************************
    * File: 1-1.c
    * Brief:
    * Author: luoyuxiang
    * Mail: 269724033@qq.com
    * Created Time: Sat 17 Dec 2016 08:00:39 PM CST
 ************************************************************************/

#include "nig_http.h"


/*
 * 系统调用read函数的包装函数
 * 相对于read，增加了内部缓冲区
 */
 ssize_t http_read(http_t *rp, char *usrbuf, size_t n)
{
    int cnt;
    // 内部缓冲区为空，从缓冲区对应的描述符中继续读取字节填满内部缓冲区
    while (rp->nig_cnt <= 0)
    {
        rp->nig_cnt = read(rp->nig_fd, rp->nig_buf, sizeof(rp->nig_buf));

        if (rp->nig_cnt < 0)
        { // 返回-1
            if (errno != EINTR) {
                return -1;
            }
        } else if (rp->nig_cnt == 0) { // EOF
            return 0;
        } else {
            rp->nig_bufptr = rp->nig_buf;
        }
    }


    // 比较调用所需的字节数n与内部缓冲区可读字节数rp->nig_cnt
    // 取其中最小值
    cnt = n;
    if (rp->nig_cnt < n) {
        cnt = rp->nig_cnt;
    }
    memcpy(usrbuf, rp->nig_bufptr, cnt);
    rp->nig_bufptr += cnt;
    rp->nig_cnt -= cnt;
    return cnt;
}
 /*
  * 从描述符fd中读取n个字节到存储器位置usrbuf
  */
 ssize_t http_readn(int fd, void *usrbuf, size_t n)
 {
     size_t nleft = n; // 剩下的未读字节数
     ssize_t nread;
     char *bufp = usrbuf;

     while (nleft > 0) {
         if ((nread = read(fd, bufp, nleft)) < 0) {
             if (errno == EINTR) { // 被信号处理函数中断返回
                 nread = 0;
             } else {
                 return -1;  // read出错
             }
         } else if (nread == 0) { // EOF
             break;
         }
         nleft -= nread;
         bufp += nread;
     }

     return (n - nleft); // 返回已经读取的字节数
 }
 /*
  * 从文件rp中读取n字节数据到usrbuf
  */
 ssize_t http_readnb(http_t *rp, void *usrbuf, size_t n)
 {
     size_t nleft = n; // 剩下的未读取字节数
     ssize_t nread;
     char *bufp = usrbuf;

     while (nleft > 0) {
         if ((nread = http_read(rp, bufp, nleft)) < 0) {
             if(errno == EINTR) { // 被信号处理程序中断返回
                 nread = 0;
             } else {
                 return -1; // 读取数据出错
             }
         } else if(nread == 0) { // EOF
             break;
         }
         nleft -= nread;
         bufp += nread;
     }

     return (n - nleft);
 }




/*
 * 将usrbuf缓冲区中的前n个字节数据写入fd中
 * 该函数会保证n个字节都会写入fd中
 */
ssize_t http_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n; // 剩下的未写入字节数
    ssize_t nwritten;
    char *bufp = (char *)usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno == EINTR) { // 被信号处理函数中断返回
                nwritten = 0;
            } else { // write函数出错
                return -1;
            }
        }
        nleft -= nwritten;
        bufp += nwritten;
    }

    return n;
}



/*
 * 初始化内部缓冲区rio_t结构
 */
void http_readinitb(http_t *rp, int fd)
{
	//memset(rp, 0, sizeof(http_t));
    rp->nig_fd = fd;
    rp->nig_cnt = 0;
    rp->nig_bufptr = rp->nig_buf;
    return ;
}


/*
 * 从文件rp中读取一行数据（包括结尾的换行符），拷贝到usrbuf
 * 并用0字符来结束这行数据
 */
ssize_t http_get_line(http_t *t, void *usrbuf, size_t maxlen)
{
    int n;
    int rc;
    char c;
    char * bufp = usrbuf;
    //write_log(("/data/cluster/web/src/node/c/nig/123.log") , "%s \r\n", "--99999");

    for (n = 1; n < maxlen; n++) //循环2048次
    {
        if ((rc = http_read(t, &c, 1)) == 1) {
            *bufp++ = c;
            if (c == '\n') { // 读完了一行
                break;
            }
        } else if (rc == 0) {
            if (n == 1) {
                return 0; // EOF,但没有读取任何数据
            } else {
                break; //EOF,但已经读取了一些数据
            }
        } else { // 出错
            return -1;
        }
    }

    *bufp = 0;
    return n;
}


void http_get_headers(http_t *rp, http_ret *ret)
{
    char buf[MAXLINE];
    char *start;
    char *end;

    memset(buf, 0, sizeof(buf));
    if (http_get_line(rp, buf, MAXLINE) < 0) {
        //error_log("http_get_line error", DEBUGARGS);
    }

    while (strcmp(buf, "\r\n")) {
        start = index(buf, ':');
        // 每行数据包含\r\n字符，需要删除
        end = index(buf, '\r');

        if (start != 0 && end != 0) {
            *end = '\0';
            while ((*(start + 1)) == ' ') {
                start++;
            }

            if (is_contype(buf)) {
                strcpy(ret->contype, start + 1);
            } else if (is_conlength(buf)) {
                strcpy(ret->conlength, start + 1);
            }
        }

        memset(buf, 0, 2048);
        if (http_get_line(rp, buf, MAXLINE) < 0) {
            //error_log("http_get_line error", DEBUGARGS);
        }
    }

    return ;
}



/*
 * 分析请求uri，提取具体文件名和查询参数
 * 请求的是静态文件返回1
 * 请求的是动态文件返回0
 * 默认的服务器根目录就是程序所在目录
 * 默认页面是index.html
 */
int http_parse_uri(char *uri, char *filename, char *name, char *cgiargs)
{
    char *ptr, *query;;
    char urin[1024];
    char *delim = ".php"; // 根据后缀名判断是静态页面还是动态页面
    char cwd[1024];
    char *dir;

    strcpy(urin, uri); // 不破坏原始字符串

    if (!(query = strstr(urin, delim))) { // 静态页面
        strcpy(cgiargs, "");

        //  删除无用参数 /index.html?123435
        ptr = index(urin, '?');
        if (ptr) {
            *ptr = '\0';
        }

        strcpy(filename, ".");
        strcat(filename, urin);
        // 如果以‘/’结尾，自动添加index.html
        if (urin[strlen(urin) - 1] == '/') {
            strcat(filename, "index.html");
        }
        return 1;
    } else { // 动态页面
        // 提取查询参数
        ptr = index(urin, '?');
        if (ptr) {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        } else {
            // 类似index.php/class/method会提取class/method的参数
            if (*(query + sizeof(delim)) == '/') {
                strcpy(cgiargs, query + sizeof(delim) + 1);
                *(query + sizeof(delim)) = '\0';
            }
        }
        dir = getcwd(cwd, 1024); // 获取当前工作目录
        strcpy(filename, dir);       // 包含完整路径名
        strcat(filename, urin);
        strcpy(name, urin);         // 不包含完整路径名
        return 0;
    }
}


void http_send_headers(int client)
{
     char buf[1024];
     strcpy(buf, "HTTP/1.0 200 OK\r\n");
     send(client, buf, strlen(buf), 0);
     strcpy(buf, "Server: httpd/0.1.0\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "Content-Type: text/html\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "tinyhttp: server123456\r\n");
     send(client, buf, strlen(buf), 0);
     //strcpy(buf, "\r\n");
     //send(client, buf, strlen(buf), 0);
}


void http_cat(int client, FILE *resource)
{
     char buf[1024];
     //从文件文件描述符中读取指定内容
     fgets(buf, sizeof(buf), resource);
     while (!feof(resource))
     {
          send(client, buf, strlen(buf), 0);
          fgets(buf, sizeof(buf), resource);
     }
}


void http_unimplemented(int client)
{
     char buf[1024];
     sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "Server: httpd/0.1.0\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "Content-Type: text/html\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "</TITLE></HEAD>\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "</BODY></HTML>\r\n");
     send(client, buf, strlen(buf), 0);
}



void http_not_found(int client)
{
     char buf[1024];
     sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "Server: httpd/0.1.0\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "Content-Type: text/html\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "<HTML><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><TITLE>Not Found</TITLE></head>\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "<BODY><P>没有找到文件\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "your request because the resource specified\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "is unavailable or nonexistent.\r\n");
     send(client, buf, strlen(buf), 0);
     sprintf(buf, "</BODY></HTML>\r\n");
     send(client, buf, strlen(buf), 0);
}



void http_error_die(const char *sc)
{
     //包含于<stdio.h>,基于当前的 errno 值，在标准错误上产生一条错误消息。参考《TLPI》P49
     perror(sc);
     exit(1);
}
