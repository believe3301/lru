#include "lru.h"

unsigned int hashpower = 32;

#define hashsize(n) ((size_t)1<<(n))
#define hashmask(n) (hashsize(n)-1)

static lru_item** table;
static unsigned int hash_items = 0;

void 
hash_init(const int hashpower_init)
{
    if (hashpower_init > 0) {
        hashpower = hashpower_init;
    }
    table = calloc(hashsize(hashpower), sizeof(void *));
}

lru_item*
hash_find(const char *key, const size_t nkey, const uint32_t hv)
{
    lru_item *it = table[hv & hashmask(hashpower)];
    lru_item *ret = NULL;
    while (it) {
        if ((nkey == it->nkey) && (memcmp(key, ITEM_key(it), nkey) == 0)) {
            ret = it;
            break;
        }
        it = it->h_next;
    }
    return ret;
}

int 
hash_insert(lru_item *it, const uint32_t hv)
{
    it->h_next = table[hv & hashmask(hashpower)];
    table[hv & hashmask(hashpower)] = it;
    hash_items++;
    return 1;
}

static lru_item** 
hashitem_before (const char *key, const size_t nkey, const uint32_t hv) 
{
    lru_item **pos;

    pos = &table[hv & hashmask(hashpower)];

    while (*pos && ((nkey != (*pos)->nkey) || memcmp(key, ITEM_key(*pos), nkey))) {
        pos = &(*pos)->h_next;
    }
    return pos;
}

void 
hash_delete(const char *key, const size_t nkey, const uint32_t hv)
{
    lru_item **before = hashitem_before(key, nkey, hv);

    if (*before) {
        lru_item *nxt;
        hash_items--;
        nxt = (*before)->h_next;
        (*before)->h_next = 0;   /* probably pointless, but whatever. */
        *before = nxt;
    }
}

void
hash_free(void)
{
    free(table);
}
