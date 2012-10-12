#include "ngx_config.h"
#include "ngx_conf_file.h"
#include "nginx.h"
#include "ngx_core.h"
#include "ngx_string.h"
#include "ngx_palloc.h"
#include "ngx_array.h"
#include "ngx_hash.h"

#include <stdio.h>

/* 避免加载整个NginX日志子系统而定义的空日志核心函数 */
volatile ngx_cycle_t  *ngx_cycle;
void ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err, const char *fmt, ...) { }


static ngx_str_t names[] = {ngx_string("Sneijder"), ngx_string("Cassano"), ngx_string("Milito")};
static char* descs[] = {"Sneijder's number is 10","Cassano's number is 99","Milito's number is 22"};


int main()
{
    ngx_uint_t          k;
    ngx_pool_t*         pool;
    ngx_hash_init_t     hash_init;
    ngx_hash_t*         hash;
    ngx_array_t*        elements;
    ngx_hash_key_t*     arr_node;
    char*               find;
    int                 i;
    u_char              lowcase_str[64] = {0};

    ngx_cacheline_size  = 32;

    /* hash key cal test*/
    ngx_str_t str       = ngx_string("hello, world");
    k                   = ngx_hash_key_lc( str.data, str.len);
    printf("caculated key is %u \n", k);

    pool = ngx_create_pool(1024*10, NULL);
    hash = (ngx_hash_t*) ngx_pcalloc(pool, sizeof(hash));

    hash_init.hash        = hash;
    hash_init.key         = &ngx_hash_key_lc;
    hash_init.max_size    = 1024*10;
    hash_init.bucket_size = ngx_align(64, ngx_cacheline_size);
    hash_init.name        = "interfc_player_hash";
    hash_init.pool        = pool;
    hash_init.temp_pool   = NULL;

    elements = ngx_array_create(pool, 32, sizeof(ngx_hash_key_t));
    for(i = 0; i < 3; i++) {
        arr_node            = (ngx_hash_key_t*) ngx_array_push(elements);
        arr_node->key       = (names[i]);
        arr_node->key_hash  = ngx_hash_key_lc(arr_node->key.data, arr_node->key.len);
        arr_node->value     = (void*) descs[i];

        printf("key: %s , key_hash: %u\n", arr_node->key.data, arr_node->key_hash);
    }

    /* 注意：保存到ngx_hash_elt_t的name都转化成小写了 */
    if (ngx_hash_init(&hash_init, (ngx_hash_key_t*) elements->elts, elements->nelts) != NGX_OK){
        return 1;
    }

    /* 这里names中包含大写字幕 */
    k    = ngx_hash_key_lc(names[0].data, names[0].len);
    printf("%s key is %d\n", names[0].data, k); 

    /* 转成小写来查找 */
    ngx_strlow(lowcase_str, names[0].data, names[0].len);
    find = (char*)ngx_hash_find(hash, k, lowcase_str, names[0].len);

    if (find) {
        printf("get desc : %s\n", (char*) find);
    }

    ngx_array_destroy(elements);
    ngx_destroy_pool(pool);

    return 0;
}