#include <stdio.h>
#include <assert.h>
#include "lru.h"

int main(void)
{
    lru_init(0);
    item_set("key1", 4, "abcd", 4);

    char *value = item_get("key1", 4);
    printf("key: %s, value: %s\n", "key1", value);

    item_delete("key1", 4);
    value = item_get("key1", 4);
    assert(value == NULL);

    return 0;
}
