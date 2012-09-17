lru_item* hash_find(lru *l, const char *key, const size_t nkey, const uint32_t hv);

int hash_insert(lru *l, lru_item *it, const uint32_t hv);

void hash_delete(lru *l, const char *key, const size_t nkey, const uint32_t hv);
