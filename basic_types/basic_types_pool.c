#include "ngx_config.h"
#include "ngx_conf_file.h"
#include "nginx.h"
#include "ngx_core.h"
#include "ngx_string.h"
#include "ngx_palloc.h"

#include <stdio.h>

/* 避免加载整个NginX日志子系统而定义的空日志核心函数 */
volatile ngx_cycle_t  *ngx_cycle;
void ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err, const char *fmt, ...) { }

typedef struct example_s {
    int a;
    char* b;
} example_t;

int main()
{
    ngx_pool_t *pool = NULL;
    example_t  *exap = NULL;
    char       *str  = NULL;

    pool = ngx_create_pool(5000, NULL);
    if(pool == NULL){
        printf("%s\n", "exec ngx_create_pool return NULL.");
        return 1;
    }
    printf("available pool regular pool free size is %d now\n", (ngx_uint_t) (pool->d.end - pool->d.last));
    
    exap = ngx_palloc(pool, sizeof(example_t)) ;
    str  = ngx_palloc(pool, sizeof("hello,world"));
    printf("available pool regular pool free size is %d now\n", (ngx_uint_t) (pool->d.end - pool->d.last));

    exap->a = 1;
    exap->b = str;
    strcpy(str, "hello,world");
    printf("pool max is %d\n", pool->max);
    printf("exap->a is %d, exap->b is %s\n", exap->a, exap->b);

    ngx_destroy_pool(pool);
    return 0;
}