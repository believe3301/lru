#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAXBYTE_DEDAULT 64 * 1024 * 1024 /* 64M */

#define ITEM_size   (sizeof(lru_item))

#define ITEM_key(item) ((char*)((item)->data))    /* terminal null */

#define ITEM_data(item) (char*) ((item)->data) + (item)->nkey + 1 

/* item */
typedef struct lru_item_ {
    struct lru_item_ *next;
    struct lru_item_ *prev;
    struct lru_item_ *h_next;   /* hash chain next */
    int nbytes;         /* size of total items, ITEM_size + nkey + 1 + nvalue*/
    uint8_t nkey;       /* key len, not include terminal null */
    char data[];        /* key and value, include terminal null */
}lru_item;


/* stat */
typedef struct lru_stat {
    uint64_t max_bytes;

    uint64_t total_items;
    uint64_t curr_items;

    uint64_t total_bytes;
    uint64_t malloc;
    uint64_t malloc_failed;

    uint64_t free_bytes;
    uint64_t free;
    uint64_t curr_bytes;
    
    uint64_t get_cmds;
    uint64_t get_hits;
    uint64_t get_misses;

    uint64_t set_cmds;
    uint64_t set_failed;

    uint64_t del_cmds;
    uint64_t del_hits;
    uint64_t del_misses;


    uint64_t evictions;
}lru_stat;

#include "hash.h"

void lru_init(size_t maxbytes);

void* item_get(const char *key, const size_t nkey);

/* 0 success , 1 failed */
int item_set(const char *key, const size_t nkey, const char *value, const size_t nvalue);

void item_delete(const char *key, const size_t nkey);

void print_stat(void);

void lru_free(void);
