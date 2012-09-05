#include "lru.h"

#include <assert.h>

static lru_item *head;
static lru_item *tail;
static lru_stat *stat;

void 
lru_init(const size_t maxbytes)
{
    head = NULL;
    tail = NULL;
    stat = malloc(sizeof(lru_stat));
    memset(stat, 0, sizeof(lru_stat));
    
    if (maxbytes < 1 * 1024 * 1024) {
        stat->max_bytes = MAXBYTE_DEDAULT;
    } else {
        stat->max_bytes = maxbytes;
    }

    hash_init(0);
}

//TODO 
static uint32_t 
hash(const char *key, const size_t nkey)
{
  const char *p;
  uint32_t h = 5381;

  for (p = key; p <= key + nkey; p += 1)
    h = (h << 5) + h + *p;
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

    stat->curr_items --;
    stat->curr_bytes -= it->nbytes;
    stat->free_bytes += it->nbytes;
    stat->free++;
    free(it);
}

static void
do_item_remove(lru_item *it)
{
    uint32_t hv = hash(ITEM_key(it), it->nkey);
    do_item_remove_hv(it, hv);
}

void* 
item_get(const char *key, const size_t nkey)
{
    stat->get_cmds++;
    uint32_t hv = hash(key, nkey);
    lru_item *it = do_item_get(key, nkey, hv);
    if (it != NULL) {
        stat->get_hits ++;
        return ITEM_data(it);
    }
    stat->get_misses ++;
    return NULL;
}

static void*
item_alloc(const size_t sz, lru_item *old)
{
    void *m = NULL;
    int delta = old ? old->nbytes : 0;
    if ((stat->curr_bytes + sz - delta) <= stat->max_bytes) {
        
        stat->malloc ++;
        m = malloc(sz);
        if (!m) {
            stat->malloc_failed += 1;
            return NULL;
        }

    } else if (sz > stat->max_bytes) {
        return NULL;

    } else {
        //evict 
        assert(tail != NULL);
        lru_item *it = tail;
        while((it != NULL) && (stat->curr_bytes + sz - delta) > stat->max_bytes) {
            if (it != old) {
                do_item_remove(it);
            }
            stat->evictions ++;
            it = tail;
        }
        m = malloc(sz);
        if (!m) {
            stat->malloc_failed += 1;
            return NULL;
        }
    }
    stat->curr_bytes += sz;
    stat->total_bytes += sz;
    stat->curr_items ++;
    stat->total_items ++;
    return m;
}

int 
item_set(const char *key, const size_t nkey, const char *value, const size_t nvalue)
{
    stat->set_cmds ++;

    uint32_t hv = hash(key, nkey);
    lru_item *old = do_item_get(key, nkey, hv);

    size_t isize = sizeof(lru_item) + nkey + 1 + nvalue;
    lru_item *it = item_alloc(isize, old);
    if (it == NULL) {
        stat->set_failed ++;
        return 1;
    }

    if (old != NULL) {
        do_item_remove_hv(old, hv);
    }

    it->nkey = nkey;
    it->nbytes = nkey + 1 + nvalue;

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

void 
item_delete(const char *key, const size_t nkey)
{
    stat->del_cmds++;
    uint32_t hv = hash(key, nkey);
    lru_item *it = hash_find(key, nkey, hv);

    if (it != NULL) {
        do_item_remove_hv(it, hv);
        stat->del_hits++;
    }
    stat->del_misses++;
}

void 
print_stat(void)
{
    

}

void 
lru_free(void)
{

}
