#include "lru.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

static lru_item *head = NULL;
static lru_item *tail = NULL;
lru_stat stat;

int 
lru_init(const size_t maxbytes)
{
    head = NULL;
    tail = NULL;
    
    if (maxbytes <= 0) {
        stat.max_bytes = MAXBYTE_DEDAULT;
    } else {
        stat.max_bytes = maxbytes;
    }

    return hash_init(0);
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
do_item_get(const char *key, const size_t nkey, const uint32_t hv)
{
    return hash_find(key, nkey, hv);
}

static void
do_item_remove_hv(lru_item *it, const uint32_t hv) 
{
    assert(it != NULL);
    hash_delete(ITEM_key(it), it->nkey, hv);
    if (head == it) {
        assert(it->prev == 0);
        head = it->next;
    }
    if (tail == it) {
        assert(it->next == 0);
        tail = it->prev;
    }
    assert(it->next != it);
    assert(it->prev != it);

    if (it->next) it->next->prev = it->prev;
    if (it->prev) it->prev->next = it->next;

    stat.curr_items --;
    stat.curr_bytes -= it->nbytes;
    stat.free_bytes += it->nbytes;
    stat.free++;
    free(it);
}

static void
do_item_remove(lru_item *it)
{
    uint32_t hv = hash(ITEM_key(it), it->nkey);
    do_item_remove_hv(it, hv);
}

int
item_get(const char *key, const size_t nkey, char *buf, const size_t nbuf, size_t *nvalue)
{
    stat.get_cmds++;
    uint32_t hv = hash(key, nkey);
    lru_item *it = do_item_get(key, nkey, hv);
    if (it != NULL) {
        stat.get_hits ++;
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
    stat.get_misses ++;
    return 1;
}

static void*
item_alloc(const size_t sz, lru_item *old)
{
    void *m = NULL;
    int delta = old ? old->nbytes : 0;
    if ((stat.curr_bytes + sz - delta) <= stat.max_bytes) {
        
        m = malloc(sz);
        if (!m) {
            stat.malloc_failed += 1;
            return NULL;
        }

    } else if (sz > stat.max_bytes) {
        return NULL;

    } else {
        //evict 
        assert(tail != NULL);
        lru_item *it = tail;
        while((it != NULL) && (stat.curr_bytes + sz - delta) > stat.max_bytes) {
            if (it != old) {
                do_item_remove(it);
                it = tail;
            } else {
                it = tail->prev;
            }

            stat.evictions ++;
        }
        m = malloc(sz);
        if (!m) {
            stat.malloc_failed += 1;
            return NULL;
        }
    }
    stat.malloc ++;
    stat.curr_bytes += sz;
    stat.total_bytes += sz;
    stat.curr_items ++;
    stat.total_items ++;
    return m;
}

int 
item_set(const char *key, const size_t nkey, const char *value, const size_t nvalue)
{
    stat.set_cmds ++;

    uint32_t hv = hash(key, nkey);
    lru_item *old = do_item_get(key, nkey, hv);

    size_t isize = ITEM_size + nkey + 1 + nvalue;
    lru_item *it = item_alloc(isize, old);
    if (it == NULL) {
        stat.set_failed ++;
        return 1;
    }

    if (old != NULL) {
        do_item_remove_hv(old, hv);
    }

    it->nkey = nkey;
    it->nbytes = isize;

    memcpy(ITEM_key(it), key, nkey);
    memcpy(ITEM_data(it), value, nvalue);
    
    hash_insert(it, hv);
    it->prev = 0;
    it->next = head;
    if (it->next) it->next->prev = it;
    head = it;
    if (tail == NULL) tail = it;
    return 0;
}

int 
item_delete(const char *key, const size_t nkey)
{
    stat.del_cmds++;
    uint32_t hv = hash(key, nkey);
    lru_item *it = hash_find(key, nkey, hv);

    if (it != NULL) {
        do_item_remove_hv(it, hv);
        stat.del_hits++;
        return 0;
    } else {
        stat.del_misses++;
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
stat_print(char *buf, const int nbuf)
{
    int remaining = nbuf;

    append_stat(&buf, &remaining, "max_bytes","%llu",stat.max_bytes);
    append_stat(&buf, &remaining, "total_items","%llu",stat.total_items);
    append_stat(&buf, &remaining, "curr_items","%llu",stat.curr_items);
    append_stat(&buf, &remaining, "total_bytes","%llu",stat.total_bytes);
    append_stat(&buf, &remaining, "curr_bytes","%llu",stat.curr_bytes);
    append_stat(&buf, &remaining, "malloc","%llu",stat.malloc);
    append_stat(&buf, &remaining, "malloc_failed","%llu",stat.malloc_failed);
    append_stat(&buf, &remaining, "free_bytes","%llu",stat.free_bytes);
    append_stat(&buf, &remaining, "free","%llu",stat.free);

    append_stat(&buf, &remaining, "get_cmds","%llu",stat.get_cmds);
    append_stat(&buf, &remaining, "get_hits","%llu",stat.get_hits);
    append_stat(&buf, &remaining, "get_misses","%llu",stat.get_misses);

    append_stat(&buf, &remaining, "set_cmds","%llu",stat.set_cmds);
    append_stat(&buf, &remaining, "set_failed","%llu",stat.set_failed);

    append_stat(&buf, &remaining, "del_cmds","%llu",stat.del_cmds);
    append_stat(&buf, &remaining, "del_hits","%llu",stat.del_hits);
    append_stat(&buf, &remaining, "del_misses","%llu",stat.del_misses);

    append_stat(&buf, &remaining, "hash_power_level","%u",stat.hash_power_level);
    append_stat(&buf, &remaining, "hash_find_depth","%u",stat.hash_find_depth);
    append_stat(&buf, &remaining, "hash_bytes","%llu",stat.hash_bytes);

    append_stat(&buf, &remaining, "evictions","%llu",stat.evictions);
}

void stat_reset(void)
{
    size_t max_bytes = stat.max_bytes;
    memset(&stat, 0, sizeof(struct lru_stat));
    stat.max_bytes = max_bytes;
}

void 
lru_free(void)
{
    //free items
    lru_item *it;
    it = head;
    while (it != NULL) {
        do_item_remove(it);
        it = head;
    }
    assert(head == NULL);
    assert(tail == NULL);
    //free hashtable
    hash_free();
}
