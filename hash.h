void hash_init(const int hashpower_init);

lru_item* hash_find(const char *key, const size_t nkey, const uint32_t hv);

int hash_insert(lru_item *it, const uint32_t hv);

void hash_delete(const char *key, const size_t nkey, const uint32_t hv);

