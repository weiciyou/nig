/*************************************************************************
    * File: 1-1.c
    * Brief:
    * Author: luoyuxiang
    * Mail: 269724033@qq.com
    * Created Time: Sat 17 Dec 2016 08:00:39 PM CST
 ************************************************************************/

#ifndef _NIG_PROGRESS_H_INCLUDED_
#define _NIG_PROGRESS_H_INCLUDED_

#include "nig_header.h"
#include "nig_config.h"

typedef struct {
    char chr;		/*提示字符*/
    char *title;	/*提示字符串*/
    int style;		/*进步的风格*/
    int max;		/*最大值*/
    float offset;
    char *pro;
} progress_t;

#define PROGRESS_NUM_STYLE 0
#define PROGRESS_CHR_STYLE 1
#define PROGRESS_BGC_STYLE 2

NIG_API void progress_init(progress_t *, char *, int, int);

NIG_API void progress_show(progress_t *, float);

NIG_API void progress_destroy(progress_t *);

#endif	/*ifndef*/
