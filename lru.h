#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX(a, b) (((a) > (b))?(a) : (b))
#define MIN(a, b) (((a) > (b))?(b) : (a))
#define MAXBYTE_DEDAULT 64 * 1024 * 1024 /* 64M */
#define HASH_POWER_DEFAULT 16

#define hashsize(n) ((size_t)1<<(n))
#define hashmask(n) (hashsize(n)-1)


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

    unsigned int hash_find_depth;
    uint64_t hash_bytes;
    

    uint64_t evictions;
}lru_stat;

typedef struct lru{
    unsigned int hashpower;
    uint64_t max_bytes;  /* max used bytes, include item size, not include hash bytes */

    lru_item **table;   /* hash */
    lru_item *head;     /* double link head */
    lru_item *tail;     /* double link tail */

    lru_stat stat;      /* stat */
}lru;


#include "hash.h"

lru* lru_init(const uint64_t maxbytes, const unsigned int hashpower);

/*
   0 success 
   1 failed 
   alloc value buf by caller or add refcount by item...
*/
int item_get(lru *l, const char *key, const size_t nkey, char *buf, const size_t nbuf, size_t *nvalue);

/* 0 success , 1 failed */
int item_set(lru *l, const char *key, const size_t nkey, const char *value, const size_t nvalue);

/* 0 hit, 1 miss */
int item_delete(lru *l, const char *key, const size_t nkey);

#define stat_reset(l) do { \
    memset(&(l->stat), 0, sizeof(struct lru_stat)); \
} while(0)

void stat_print(lru *l, char *buf, const int nbuf);

void lru_free(lru *l);
