/*************************************************************************
    * File: 1-1.c
    * Brief:
    * Author: luoyuxiang
    * Mail: 269724033@qq.com
    * Created Time: Sat 17 Dec 2016 08:00:39 PM CST
 ************************************************************************/

#ifndef _NIG_H_CONFIG_
#define _NIG_H_CONFIG_
//version of the celib
#define _NIG_VERSION 1.1
#define nig_version() _NIG_VERSION

#if ( defined(WIN32) || defined(_WIN32) || defined(WINNT) )
#    define NIG_API         extern __declspec(dllexport)
#    define NIG_STATIC     static
#else
#    define NIG_API         extern
#    define NIG_STATIC     static inline
#endif


//短的数据类型
typedef long long int                     llong_t;
typedef unsigned long long int           ullong_t;

typedef long int                           long_t;
typedef unsigned long int                 ulong_t;

typedef unsigned int                       uint_t;
typedef unsigned short                   ushort_t;


typedef int *                           nig_int_t;
typedef unsigned int *                 nig_uint_t;
typedef char *                         nig_char_t;

#ifndef LOGS
#define LOGS(...) fprintf(stderr, "c *** %s: ", __func__); \
            fprintf(stderr, ##__VA_ARGS__); \
            fprintf(stderr, " at %s line %d.\n", __FILE__, __LINE__)
#endif

//判断是否为空
#define ISspace(x) isspace((int)(x))



#define MAXLINE 8192 // 最大行长度

#endif /* _NIG_H_CONFIG_ */
