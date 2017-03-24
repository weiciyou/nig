/*************************************************************************
    * File: 1-1.c
    * Brief:
    * Author: luoyuxiang
    * Mail: 269724033@qq.com
    * Created Time: Sat 17 Dec 2016 08:00:39 PM CST
 ************************************************************************/

#include "nig_http.h"

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
 * 将usrbuf缓冲区中的前n个字节数据写入fd中
 * 该函数会保证n个字节都会写入fd中
 */
ssize_t http_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n; // 剩下的未写入字节数
    ssize_t nwritten;
    char *bufp = (char *)usrbuf;

    while (nleft > 0)
    {
        if ((nwritten = write(fd, bufp, nleft)) <= 0)
        {
        	// 被信号处理函数中断返回
            if (errno == EINTR)
            {
                nwritten = 0;
            }
            else
            { // write函数出错
                return -1;
            }
        }
        nleft -= nwritten;
        bufp += nwritten;
    }

    return n;
}

 ssize_t http_get_line_t(int sock, char *buf, int size)
 {
	 int i = 0;
	 char c = '\0';
	 int n;
	 while ((i < size - 1) && (c != '\n'))
	 {
		 //读一个字节的数据存放在 c 中
		 n = recv(sock, &c, 1, 0);

		 if (n > 0) {
			 if (c == '\r') {
			 	 n = recv(sock, &c, 1, MSG_PEEK);
			 	 if ((n > 0) && (c == '\n')) {
			 		 recv(sock, &c, 1, 0);
			 	 }else {
			 		 c = '\n';
			 	 }
			 }

			 buf[i] = c;
			 i++;
		 } else {
			 c = '\n';
		 }
	 }
	 buf[i] = '\0';
	 return i;
}

void http_get_headers(int fd, http_ret *ret)
{
    char buf[MAXLINE];
    char *start;
    char *end;

	memset(buf, 0, sizeof(buf));
	if (http_get_line_t(fd, &buf[0], sizeof(buf)) < 0)
	{
		 perror("do_read 函数错误");
	}
	LOGS("请求第2行---> %s <--" , buf);


	 while(buf[0] && strlen(buf) > 0)
     {
		start = index(buf, ':');
		// 每行数据包含\r\n字符，需要删除
		end = index(buf, '\n');

		if (start != 0 && end != 0) {
			*end = '\0';
			while ((*(start + 1)) == ' ') {
				start++;
			}

			if (is_contype(buf))
			{
				strcpy(ret->contype, start + 1);
			}
			if (is_conlength(buf))
			{
				strcpy(ret->conlength, start + 1);
			}
		}

		if ((start ==0) && (strlen(buf) >= 3))  //post内容
		{
			ret->args_post = (char*)malloc(sizeof(buf));
			strcpy(ret->args_post, buf);
		}

		memset(buf, 0, sizeof(buf));
		if (http_get_line_t(fd, &buf[0], sizeof(buf)) < 0)
		{
			 perror("do_read 函数错误");
		}
		LOGS("请求第3行---> %s <--" , buf);
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
     return ;
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
     sprintf(buf, "<BODY><P>没有找到文件</BODY></HTML>\r\n");
     send(client, buf, strlen(buf), 0);
}
