#include "lru.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

lru* 
lru_init(const uint64_t maxbytes, const unsigned int hashpower)
{
    lru *l = calloc(sizeof(lru), 1);
    
    if (l == NULL) {
        fprintf(stderr, "lru init alloc lru failed.\n");
        return NULL;
    }

    if (hashpower > 32) {
        fprintf(stderr, "hash power must less than 32.\n");
        return NULL;
    } else {
        l->hashpower = hashpower == 0 ? HASH_POWER_DEFAULT: hashpower;
    }

    l->max_bytes = maxbytes ==0 ? MAXBYTE_DEDAULT: maxbytes;
    l->table = calloc(hashsize(l->hashpower), sizeof(void *));

    if (l->table == NULL) {
        fprintf(stderr, "lru init alloc hash buckets failed.\n");
        free(l);
        return NULL;
    }

    l->stat.hash_bytes = hashsize(l->hashpower);
    return l;
}

static uint32_t 
hash(const char *key, const int nkey)
{
  const char *p;
  int i;
  uint32_t h = 5381;

  for(i = 0, p = key; i < nkey; p++, i++) {
    h = (h << 5) + h + *p;
  }

  return h;
}

static inline lru_item*
do_item_get(lru *l, const char *key, const size_t nkey, const uint32_t hv)
{
    return hash_find(l, key, nkey, hv);
}

static void
do_item_remove_hv(lru *l, lru_item *it, const uint32_t hv) 
{
    assert(it != NULL);
    hash_delete(l, ITEM_key(it), it->nkey, hv);
    if (l->head == it) {
        assert(it->prev == 0);
        l->head = it->next;
    }
    if (l->tail == it) {
        assert(it->next == 0);
        l->tail = it->prev;
    }
    assert(it->next != it);
    assert(it->prev != it);

    if (it->next) it->next->prev = it->prev;
    if (it->prev) it->prev->next = it->next;

    l->stat.curr_items --;
    l->stat.curr_bytes -= it->nbytes;
    l->stat.free_bytes += it->nbytes;
    l->stat.free++;
    free(it);
}

static void
do_item_remove(lru *l, lru_item *it)
{
    uint32_t hv = hash(ITEM_key(it), it->nkey);
    do_item_remove_hv(l, it, hv);
}

int
item_get(lru *l, const char *key, const size_t nkey, char *buf, const size_t nbuf, size_t *nvalue)
{
    l->stat.get_cmds++;
    uint32_t hv = hash(key, nkey);
    lru_item *it = do_item_get(l, key, nkey, hv);
    if (it != NULL) {
        l->stat.get_hits ++;
        size_t vlen = it->nbytes - ITEM_size - it->nkey - 1;

        if (nvalue) {
            *nvalue = vlen;
        }

        if (buf && nbuf) {
            void *data = ITEM_data(it);
            memcpy(buf, data, MIN(nbuf, vlen));
        }
        return 0;
    }
    l->stat.get_misses ++;
    return 1;
}

static void*
item_alloc(lru *l, const size_t sz, lru_item *old)
{
    void *m = NULL;
    int delta = old ? old->nbytes : 0;
    if ((l->stat.curr_bytes + sz - delta) <= l->max_bytes) {
        
        m = malloc(sz);
        if (!m) {
            l->stat.malloc_failed += 1;
            return NULL;
        }

    } else if (sz > l->max_bytes) {
        return NULL;

    } else {
        //evict 
        assert(l->tail != NULL);
        lru_item *it = l->tail;
        while((it != NULL) && (l->stat.curr_bytes + sz - delta) > l->max_bytes) {
            if (it != old) {
                do_item_remove(l, it);
                it = l->tail;
            } else {
                it = l->tail->prev;
            }

            l->stat.evictions ++;
        }
        m = malloc(sz);
        if (!m) {
            l->stat.malloc_failed += 1;
            return NULL;
        }
    }
    l->stat.malloc ++;
    l->stat.curr_bytes += sz;
    l->stat.total_bytes += sz;
    l->stat.curr_items ++;
    l->stat.total_items ++;
    return m;
}

int 
item_set(lru *l, const char *key, const size_t nkey, const char *value, const size_t nvalue)
{
    l->stat.set_cmds ++;

    uint32_t hv = hash(key, nkey);
    lru_item *old = do_item_get(l, key, nkey, hv);

    size_t isize = ITEM_size + nkey + 1 + nvalue;
    lru_item *it = item_alloc(l, isize, old);
    if (it == NULL) {
        l->stat.set_failed ++;
        return 1;
    }

    if (old != NULL) {
        do_item_remove_hv(l, old, hv);
    }

    it->nkey = nkey;
    it->nbytes = isize;

    memcpy(ITEM_key(it), key, nkey);
    memcpy(ITEM_data(it), value, nvalue);
    
    hash_insert(l, it, hv);
    it->prev = 0;
    it->next = l->head;
    if (it->next) it->next->prev = it;
    l->head = it;
    if (l->tail == NULL) l->tail = it;
    return 0;
}

int 
item_delete(lru *l, const char *key, const size_t nkey)
{
    l->stat.del_cmds++;
    uint32_t hv = hash(key, nkey);
    lru_item *it = hash_find(l, key, nkey, hv);

    if (it != NULL) {
        do_item_remove_hv(l, it, hv);
        l->stat.del_hits++;
        return 0;
    } else {
        l->stat.del_misses++;
        return 1;
    }
}

static void 
append_stat(char **buf, int *nbuf, char *name, const char *fmt, ...)
{

    if (*nbuf <= 0) {
        return;
    }

    char val_str[16];
    int len;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(val_str, sizeof(val_str) - 1, fmt, ap);
    va_end(ap);

    len = snprintf(*buf, *nbuf, "%s = %s\r\n", name, val_str);
    *nbuf -= len;
    *buf += len;
}

void 
stat_print(lru *l, char *buf, const int nbuf)
{
    int remaining = nbuf;

    append_stat(&buf, &remaining, "max_bytes","%llu", l->max_bytes);
    append_stat(&buf, &remaining, "total_items","%llu", l->stat.total_items);
    append_stat(&buf, &remaining, "curr_items","%llu", l->stat.curr_items);
    append_stat(&buf, &remaining, "total_bytes","%llu", l->stat.total_bytes);
    append_stat(&buf, &remaining, "curr_bytes","%llu", l->stat.curr_bytes);
    append_stat(&buf, &remaining, "malloc","%llu", l->stat.malloc);
    append_stat(&buf, &remaining, "malloc_failed","%llu", l->stat.malloc_failed);
    append_stat(&buf, &remaining, "free_bytes","%llu", l->stat.free_bytes);
    append_stat(&buf, &remaining, "free","%llu", l->stat.free);

    append_stat(&buf, &remaining, "get_cmds","%llu", l->stat.get_cmds);
    append_stat(&buf, &remaining, "get_hits","%llu", l->stat.get_hits);
    append_stat(&buf, &remaining, "get_misses","%llu", l->stat.get_misses);

    append_stat(&buf, &remaining, "set_cmds","%llu", l->stat.set_cmds);
    append_stat(&buf, &remaining, "set_failed","%llu", l->stat.set_failed);

    append_stat(&buf, &remaining, "del_cmds","%llu", l->stat.del_cmds);
    append_stat(&buf, &remaining, "del_hits","%llu", l->stat.del_hits);
    append_stat(&buf, &remaining, "del_misses","%llu", l->stat.del_misses);

    append_stat(&buf, &remaining, "hash_power_level","%u", l->hashpower);
    append_stat(&buf, &remaining, "hash_find_depth","%u", l->stat.hash_find_depth);
    append_stat(&buf, &remaining, "hash_bytes","%llu", l->stat.hash_bytes);

    append_stat(&buf, &remaining, "evictions","%llu", l->stat.evictions);
}

void 
lru_free(lru *l)
{
    //free items
    lru_item *it;
    it = l->head;
    while (it != NULL) {
        do_item_remove(l, it);
        it = l->head;
    }
    assert(l->head == NULL);
    assert(l->tail == NULL);
    //free hashtable
    free(l->table);
    free(l);
}
