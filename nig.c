/*************************************************************************
    * File: 1-1.c
    * Brief:
    * Author: luoyuxiang
    * Mail: 269724033@qq.com
    * Created Time: Sat 17 Dec 2016 08:00:39 PM CST
 ************************************************************************/ 

#include "nig_alloc.h"
#include "nig_string.h"
#include "nig_progress.h"
#include "nig_http.h"
#include "nig_epoll.h"
#include "nig_fastcgi.h"
#include "nig_thrpool.h"

/* 存储配置参数 */
struct httpd_conf {
    char ip[16];
    int port;
    char fastcgi_ip[16];
    int fastcgi_port;
    int thread_num;
    int job_max_num;
    char filename[255]; /* configuration file */
};
struct httpd_conf conf;

/**
 * 启动进度条
 * @access public
 * @param void void
 * @return void
 */
void init_progress()
{
    progress_t bar;
    progress_init(&bar, "", 50, PROGRESS_CHR_STYLE);

    int i;
    for ( i = 0; i <= 50; i++ )
    {
        progress_show(&bar, i/50.0f);
        usleep(25000);//单位是微秒
    }
    printf("\n+-Done\n");
    progress_destroy(&bar);
    return ;
}

struct option long_options[] = {
     { "help",     0,        NULL,    '?'},
     { "signal",   0,        NULL,    's' },
     { "configuration", 1,   NULL,    'c' },
     {0,0,0,0}
};

static void nig_usage(void)
{
    fprintf(stderr,
            "nig [option]... URL\n"
            "  -s|--reload              重启指令.\n"
            "  -c|--filename            指定配置文件.如: /data/nig.conf  \n"
            "  -?|-h|--help             This information.\n"
            "  -h|--version             Display program version.\n"
           );
    exit(EXIT_FAILURE);
}

/**
 * 启动命令行参数解析
 * @access public
 * @param int argc    参数个数
 * @param char ** argv 参数数组每个元素都是指针
 * @param struct httpd_conf * 结构体
 * @return void
 */
void init_help(int argc, char **argv, struct httpd_conf * conf)
{
    int c;
    while((c = getopt_long (argc, argv, "s:c:?h", long_options, NULL)) != -1)
    {
        switch (c)
        {
            case  0 : break;
            case 's':
                printf("模仿nginx 重启指令 %s \n", optarg);
                break;
            case 'c':
                printf("模仿nginx配置文件指令 %s \n", optarg); //c 参数后面的内容
                strcpy(conf->filename, optarg);
                break;
            case ':':
            case 'h':
            case '?':
            	nig_usage();
                break;
        }
    }
    return ;
}

/**
 * 初始化配置文件
 * @access public
 * @param struct httpd_conf * 结构体
 * @return void
 */
void init_conf(struct httpd_conf *conf)
{
    FILE *fp = NULL;
    char line[BUFSIZ];
    char param_name[BUFSIZ];
    char param_value[BUFSIZ];

    if (*(conf->filename + 0) != 0)
    {
        fp = fopen(conf->filename, "r");
    }
    else
    {
        fp = fopen("./nig.conf", "r");
    }

    if(fp == NULL)
    {
        perror("fail to open the configuration file.");
        exit(EXIT_FAILURE);
    }

    while(fgets(line, BUFSIZ, fp) != NULL)
    {
        sscanf(line, "%s %s", param_name, param_value);

        if(strcmp(param_name, "IP") == 0)
        {
           // memcpy(conf->ip, param_value, 16); 错误
            strcpy(conf->ip, param_value);
        }
        if(strcmp(param_name, "PORT") == 0)
        {
            conf->port = atoi(param_value);
        }
        if(strcmp(param_name, "FASTCGI_IP") == 0)
        {
            strcpy(conf->fastcgi_ip, param_value);
        }

        if(strcmp(param_name, "FASTCGI_PORT") == 0)
        {
            conf->fastcgi_port = atoi(param_value);
        }

        if(strcmp(param_name, "THREAD_NUM") == 0)
        {
            conf->thread_num = atoi(param_value);
        }
        if(strcmp(param_name, "JOB_MAX_NUM") == 0)
        {
            conf->job_max_num = atoi(param_value);
        }
    }

    fclose(fp);
    return ;
}

/**
 * 读取本地静态文件
 * @access public
 * @param http_ret * 结构体指针
 * @return void
 */
void nig_file(http_ret * ret)
{
	http_send_headers(ret->fd);
    send(ret->fd, "\r\n", strlen("\r\n"), 0);
	send(ret->fd, "hello world", strlen("hello world"), 0);
	//LOGS("断点错误 ---> %d <--" , 2);
    /* 写完就直接关了 不用再加入监听*/
    epoll_delete_event(ret, EPOLLOUT);
    return ;
}


/**
 * 发送http请求行和请求体数据给fastcgi服务器
 * @access public
 * @param http_t *   结构体指针
 * @param http_ret * 结构体指针
 * @param int cgi_sock 连接cgi程序句柄
 * @return void
 */
int send_fastcgi(http_ret * ret, int cgi_sock)
{
    int requestId, i, l;

    requestId = cgi_sock;

    // params参数名
    char *paname[] = {
        "SCRIPT_FILENAME",
        "SCRIPT_NAME",
        "REQUEST_METHOD",
        "REQUEST_URI",
        "QUERY_STRING",
        "CONTENT_TYPE",
        "CONTENT_LENGTH"
    };

    // 对应上面params参数名，具体参数值所在hhr_t结构体中的偏移
    int paoffset[] = {
        (size_t) & (((http_ret *)0)->filename),
        (size_t) & (((http_ret *)0)->name),
        (size_t) & (((http_ret *)0)->method),
        (size_t) & (((http_ret *)0)->uri),
        (size_t) & (((http_ret *)0)->cgiargs),
        (size_t) & (((http_ret *)0)->contype),
        (size_t) & (((http_ret *)0)->conlength)
    };

    // 发送开始请求记录
     sendBeginRequestRecord(http_writen, cgi_sock, requestId);

    // 发送params参数
    l = sizeof(paoffset) / sizeof(paoffset[0]);
    for (i = 0; i < l; i++)
    {
    	//LOGS("-----> %p   %s <-----", paoffset[i],   (char *)(((int)ret) + paoffset[i]));
        // params参数的值不为空才发送
         //if (strlen((char *)((int)ret + paoffset[i])) > 0)
         {
            if (sendParamsRecord(http_writen, cgi_sock, requestId, paname[i],
            		strlen(paname[i]),
            		(char *)(((int)ret) + paoffset[i]),
            		strlen((char *)(((int)ret) + paoffset[i]))
             ) < 0)
            {
                return -1;
            }
         }
    }

    // 发送空的params参数
    if (sendEmptyParamsRecord(http_writen, cgi_sock, requestId) < 0)
    {
        //error_log("sendEmptyParamsRecord error", DEBUGARGS);
        return -1;
    }

    // 继续读取请求体数据
    l = atoi(ret->conlength);
    if (l > 0) { // 请求体大小大于0
        // 发送stdin数据
        if (sendStdinRecord(http_writen, cgi_sock, requestId, ret->args_post, strlen(ret->args_post)) < 0)
        {
            free(ret->args_post);
            return -1;
        }

        free(ret->args_post);
    }

    // 发送空的stdin数据
    if (sendEmptyStdinRecord(http_writen, cgi_sock, requestId) < 0)
    {
        //error_log("sendEmptyStdinRecord error", DEBUGARGS);
        return -1;
    }
    return 0;
}


/**
 * 回调函数  将php处理结果发送给客户端
 * @access public
 * @param int fd 浏览器与服务端的连接句柄
 * @param int outlen  php返回结果内容长度
 * @param char *out   php返回结果内容
 * @param int errlen  php返回错误结果内容长度
 * @param char *err   php返回错误结果内容
 * @param FCGI_EndRequestBody *endr  结束请求记录的协议体
 * @return void
 */
ssize_t send_to_cli(int fd, int outlen, char *out, int errlen, char *err, FCGI_EndRequestBody *endr)
{
    char *p;
    int n;

    http_send_headers(fd);//发送http响应头 但没有留空行 所以php可以继续发送空行

    if (outlen > 0)
    {
        //LOGS("---> %s <--" , out);
        p = index(out, '\n');
        //LOGS("---> %s %s <--" , "77777777",p);
        n = (int)(p - out);

        if (http_writen(fd, out , outlen) < 0)
        {
            return -1;
        }
    }

    if (errlen > 0)
    {
        if (http_writen(fd, err, errlen) < 0)
        {
            //error_log("rio_written error", DEBUGARGS);
            return -1;
        }
    }
}

/**
 * 接收fastcgi返回的数据
 * @access public
 * @param http_ret * 结构体指针
 * @return void
 */
int read_fastcgi(int fd, int cgi_sock)
{
    int requestId;
    requestId = cgi_sock;

    // 读取处理结果
    if (recvRecord(http_readn, send_to_cli, fd, cgi_sock, requestId) < 0)
    {
        //error_log("recvRecord error", DEBUGARGS);
        return -1;
    }
    return 0;
}


/**
 * 处理CGI请求
 * @access public
 * @param http_ret * ret 结构体指针
 * @param http_t * t     结构体指针
 * @param struct httpd_conf * conf     结构体指针
 * @return void
 */
void nig_cgi(http_ret * ret)
{
    int cgi_sock;
    // 创建一个连接到fastcgi服务器的套接字
    cgi_sock = epoll_socket_client(conf.fastcgi_ip, conf.fastcgi_port);

    // 发送http请求数据
    send_fastcgi(ret, cgi_sock);
    // 接收处理结果
    read_fastcgi(ret->fd, cgi_sock);

    LOGS("断点错误 ---> %d <--" , 4);
    /* 写完就直接关了 不用再加入监听*/
    epoll_delete_event(ret, EPOLLOUT);

    close(cgi_sock); // 关闭与fastcgi服务器连接的套接字
}

/**
 * 处理请求
 * @access public
 * @param void *req_ptr  epoll中的 结构体指针
 * @param struct httpd_conf * conf     结构体指针
 * @return void
 */
static int accept_request(void *req_ptr)
{
    struct epoll_event ev;
    http_ret * ret = (http_ret *)req_ptr;
    int fd = ret->fd;
    int is_cgi;
    char buf[MAXLINE];
    struct stat sbuf; /*文件属性 判断文件权限*/

    if (http_get_line_t(fd, &buf[0], sizeof(buf)) < 0)
    {
    	 LOGS("断点错误 ---> %d <--" , 5);
         epoll_delete_event(ret, EPOLLIN);
         perror("do_read 函数错误");
    }
    //LOGS("请求第1行---> %s <--" , buf);
    // 提取请求方法、请求URI、HTTP版本
    sscanf(buf, "%s %s %s", ret->method, ret->uri, ret->version);

    // 只接收GET和POST请求
    if (strcasecmp(ret->method, "GET") && strcasecmp(ret->method, "POST"))
    {
    	LOGS("断点错误 ---> %d <--" , 6);
        epoll_delete_event(ret, EPOLLIN);
        http_not_found(fd);
        return 0;
    }

    http_get_headers(fd, ret);
    // 分析请求uri，获得具体请求文件名和请求参数
    is_cgi = http_parse_uri(ret->uri, ret->filename, ret->name, ret->cgiargs);

    // 判断请求文件是否存在
    if (stat(ret->filename, &sbuf) < 0) {
    	LOGS("断点错误 ---> %d <--" , 1);
    	epoll_delete_event(ret, EPOLLIN);
        http_not_found(fd);
        return 0;
    }

    epoll_update_event(ret, EPOLLOUT);

    //LOGS("请求地址---> %s <--" , ret->uri);
    //LOGS("请求方法---> %s <--" , ret->method);
    //LOGS("协议版本---> %s <--" , ret->version);
    //LOGS("完整路径---> %s <--" , ret->filename);
    //LOGS("只有文件名---> %s <--" , ret->name);
    //LOGS("参数---> %s <--" , ret->cgiargs);
    //LOGS("类型---> %s <--" , ret->contype);
    //LOGS("长度---> %s <--" , ret->conlength);

    if (is_cgi)
    {
        /*
           // 判断是否是普通文件及是否有读权限
           if (!S_ISREG(sbuf.st_mode) || !(S_IRUSR & sbuf.st_mode))
           {
               //epoll_delete_event(ret->epollfd, fd, EPOLLIN);
               return ;
           }
         */
        nig_file(ret);
    }
    else
    {
       /*
          //判断是否有执行权限
           if (!S_ISREG(sbuf.st_mode) || !(S_IXUSR & sbuf.st_mode))
           {
               epoll_delete_event(ret->epollfd, fd, EPOLLIN);
               return ;
           }
        */
        nig_cgi(ret);
    }

    close(ret->fd);
    free(ret);
    return  0;
}

/**
 * 启动处理
 * @access public
 * @param struct httpd_conf * conf     配置文件结构体指针
 * @return void
 */
int run(struct httpd_conf * conf)
{
    int fd; /* 监听的文件描述符 */
    struct sockaddr_in remote;
    socklen_t  remotelen;

    /*创建listen socket 加入epoll*/
    int listenfd = epoll_socket_servers(conf->ip, conf->port);
    listen(listenfd, 20);

    int epollfd = epoll_create_event(0);

    http_ret *request = NULL;
    request = (http_ret *)malloc(sizeof(http_ret));
    request->fd = listenfd;
    request->epollfd = epollfd;

    epoll_add_event(request, EPOLLIN | EPOLLET);
    /*创建listen socket 加入epoll  end*/

    int  event_num; /**事件个数**/
    int i;
	void * pool;
    while(1)
    {
        event_num = epoll_wait(epollfd, evlist, 128, -1);
        if (event_num == -1)
        {
            perror("epoll_pwait");
            exit(EXIT_FAILURE);
        }
        //////////////多线程 start////////////
        if (event_num > 0 )
        {
        	pool = thrpool_create(event_num);
        }
        else
        {
        	continue;
        }
        //////////////多线程 end ////////////
        for (i = 0; i < event_num; ++i)
        {
           // fd = evlist[i].data.fd;
            http_ret *req = (http_ret *)evlist[i].data.ptr;
            fd = req->fd;
            if (fd == listenfd)
            {
                int conn_sock;
                while (1)
                {
                    conn_sock = accept(listenfd, (struct sockaddr *) &remote, &remotelen);
                    if (conn_sock < 0)  break;

                    http_ret * r = (http_ret *)malloc(sizeof(http_ret));
                    memset(r, 0, sizeof(http_ret));

                    strcpy(r->remote_ip, inet_ntoa(remote.sin_addr));
                    r->remote_port = remote.sin_port;
                    r->fd = conn_sock;
                    r->epollfd = epollfd;
                    epoll_add_event(r, EPOLLIN | EPOLLET);  //设置用于注测的读操作事件
                }
            }
            else
            {
                //表示对应的文件描述符可以读
                if (evlist[i].events & EPOLLIN)
                {
                	//////////////多线程 start////////////
            		 thrpool_add_task(pool, accept_request, (void *)evlist[i].data.ptr);
            		 //////////////多线程 end ////////////

                     //accept_request(evlist[i].data.ptr, conf);
                }
                else if(evlist[i].events & (EPOLLHUP | EPOLLERR))
                {
                     close(fd);
                }
            }
        }
    	thrpool_wait(pool);
    	thrpool_destroy(pool, 0);
    }

    free(request);
    return 0;
}


int main(int argc, char *argv[])
{
    memset(&conf,0, sizeof(struct httpd_conf));
    init_help(argc, argv, &conf);/*命令行参数初始化*/
    init_progress();             /*启动进度条*/
    init_conf(&conf);            /*读取解析配置文件*/

    run(&conf);                  /*启动*/
    return 1;
}



