#include <stdlib.h>
#include <stdint.h>

/* item */
typedef struct lru_item {
    lru_item *next;
    lru_item *prev;
    lru_item *h_next;   /* hash chain next */
    int nbytes;         /* size of data */
    uint8_t nkey;
    char data[];        /* key and value */
}lru_item;


/* stat */
typedef struct lru_stat {
    uint64_t total_items;
    uint64_t curr_items;

    uint64_t total_bytes;
    uint64_t malloc;
    uint64_t free_bytes;
    uint64_t free;
    uint64_t curr_bytes;
    
    uint64_t get_cmds;
    uint64_t get_hits;
    uint64_t get_misses;
    uint64_t set_cmds;
    uint64_t set_hits;
    uint64_t set_misses;

    uint64_t evictions;
}lru_stat;

void lru_init(void);

void* item_get(const char *key, const size_t nkey);

void item_set(const char *key, const size_t nkey, const char *value, const size_t nvalue);

void item_delete(const char *key, const size_t nkey);

void print_stat(void);

void lru_free(void);
