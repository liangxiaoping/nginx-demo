#include "ngx_config.h"
#include "ngx_conf_file.h"
#include "nginx.h"
#include "ngx_core.h"
#include "ngx_string.h"
#include "ngx_palloc.h"
#include "ngx_queue.h"

#include <stdio.h>

/* 避免加载整个NginX日志子系统而定义的空日志核心函数 */
volatile ngx_cycle_t  *ngx_cycle;
void ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err, const char *fmt, ...) { }

typedef struct interfc_player_s {
    ngx_uint_t    number;
    u_char*       name;
    ngx_queue_t   queue;
} interfc_player_t;

ngx_int_t number_cmp(const ngx_queue_t* p, const ngx_queue_t* q)
{
    if(p == NULL || q == NULL){
        return -1;
    }
    
    interfc_player_t *pre  = NULL;
    interfc_player_t *next = NULL;
    pre  = (interfc_player_t*) ngx_queue_data(p, interfc_player_t, queue);
    next = (interfc_player_t*) ngx_queue_data(q, interfc_player_t, queue);

    if(pre == NULL || next == NULL){
        return -1;
    }
    
    return ((pre->number > next->number) ? 1:0);
}

int main()
{
    ngx_pool_t        *pool     = NULL;
    ngx_queue_t       *p        = NULL;
    ngx_queue_t       *interfc  = NULL;
    interfc_player_t  *player   = NULL;
    int                i        = 0;
    
    pool = ngx_create_pool(1024*10, NULL);
    
    const ngx_str_t   names[] = {
        ngx_string("Sneijder"), ngx_string("Cassano"), ngx_string("Milito")
    } ;
    const ngx_uint_t numbers[]   = {10, 99, 22};

    interfc = ngx_palloc(pool, sizeof(ngx_queue_t));
    if(interfc == NULL)
        return 1;
    ngx_queue_init(interfc);

    for(i = 0; i < 3; i++)
    {
      player = (interfc_player_t*) ngx_palloc(pool, sizeof(interfc_player_t));
      if(player == NULL)
        return 1;

      player->number   = numbers[i];
      //player->name   = (u_char*) ngx_palloc(pool, (size_t) (strlen(names[i]) + 1) );
      player->name     = (u_char*) ngx_pstrdup(pool, (ngx_str_t*) &(names[i]) );

      ngx_queue_init(&player->queue);
      ngx_queue_insert_head(interfc, &player->queue);
    }

    for(p = ngx_queue_last(interfc);
        p != ngx_queue_sentinel(interfc);
        p = ngx_queue_prev(p) ) 
    {
        player = ngx_queue_data(p, interfc_player_t, queue);
        printf("No. %d player in inter is %s \n", player->number, player->name);
    }

    ngx_queue_sort(interfc, number_cmp);
    printf("sorting....\n");

    for(p = ngx_queue_prev(interfc);
        p != ngx_queue_sentinel(interfc);
        p = ngx_queue_last(p) ) 
    {
        player = ngx_queue_data(p, interfc_player_t, queue);
        printf("No. %d player in inter is %s \n", player->number, player->name);
    }

    ngx_destroy_pool(pool);
    return 0;
}