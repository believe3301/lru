#include "lru.h"

lru_item*
hash_find(lru *l, const char *key, const size_t nkey, const uint32_t hv)
{
    lru_item *it = l->table[hv & hashmask(l->hashpower)];
    lru_item *ret = NULL;
    unsigned int depth = 0;
    while (it) {
        if ((nkey == it->nkey) && (memcmp(key, ITEM_key(it), nkey) == 0)) {
            ret = it;
            break;
        }
        it = it->h_next;
        ++depth;  
    }
    l->stat.hash_find_depth = MAX(l->stat.hash_find_depth, depth);
    return ret;
}

int 
hash_insert(lru *l, lru_item *it, const uint32_t hv)
{
    it->h_next = l->table[hv & hashmask(l->hashpower)];
    l->table[hv & hashmask(l->hashpower)] = it;
    return 1;
}

static lru_item** 
hashitem_before (lru *l, const char *key, const size_t nkey, const uint32_t hv) 
{
    lru_item **pos;

    pos = &(l->table[hv & hashmask(l->hashpower)]);

    while (*pos && ((nkey != (*pos)->nkey) || memcmp(key, ITEM_key(*pos), nkey))) {
        pos = &(*pos)->h_next;
    }
    return pos;
}

void 
hash_delete(lru *l, const char *key, const size_t nkey, const uint32_t hv)
{
    lru_item **before = hashitem_before(l, key, nkey, hv);

    if (*before) {
        lru_item *nxt;
        nxt = (*before)->h_next;
        (*before)->h_next = 0;  
        *before = nxt;
    }
}
