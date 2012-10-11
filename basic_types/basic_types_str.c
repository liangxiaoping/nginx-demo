#include "ngx_config.h"
#include "ngx_conf_file.h"
#include "nginx.h"
#include "ngx_core.h"
#include "ngx_string.h"
#include "ngx_string.h"

#include <stdio.h>

volatile ngx_cycle_t  *ngx_cycle;

/* 避免加载整个NginX日志子系统而定义的空日志核心函数 */
volatile ngx_cycle_t  *ngx_cycle;
void ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err, const char *fmt, ...) { }

int main()
{
    u_char*      p        = NULL;
    ngx_uint_t   size     = 0;
    ngx_str_t    str      = ngx_null_string;
    ngx_str_t    dst      = ngx_null_string;
    ngx_str_t    src      = ngx_string("hello, world !");
    ngx_keyval_t pair     = {ngx_string("url"), ngx_string("https://www.google.com/index.php?test=1")};
    size_t       dst_len  = ngx_base64_encoded_length(src.len);
    u_char buffer[1024];

    printf("source length is %d, destination length is %d\n", src.len, dst_len );

    ngx_str_set(&str, "hello nginx !");
    printf("str is %s\n", str.data);

    ngx_snprintf(buffer, sizeof(buffer), "%V", &str);
    buffer[str.len] = '\0';
    printf("buffer is %s\n", buffer);

    p = malloc( ngx_base64_encoded_length(src.len) + 1);
    dst.data = p;
    ngx_encode_base64(&dst, &src);
    printf("source str is %s\ndestination str is %s\n", src.data, dst.data);
    free(p);

    size = pair.value.len + 2 * ngx_escape_uri(NULL, pair.value.data, pair.value.len, NGX_ESCAPE_URI);
    p = malloc (size * sizeof(u_char));
    ngx_escape_uri(p, pair.value.data, pair.value.len, NGX_ESCAPE_URI);
    printf("escaped %s is : %s (%d)\noriginal url size is %d\n", pair.key.data, p, size, pair.value.len);
    free(p);

    return 0;
}