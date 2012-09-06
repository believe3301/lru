#include <stdio.h>
#include <assert.h>
#include "lru.h"

int main(void)
{
    lru_init(0);

    item_set("key1", 4, "abcd", 5);
    item_set("key2", 4, "abcd", 5);
    item_set("key3", 4, "abcd", 5);

    char value[256];
    size_t vsz;

    item_get("key1", 4, value, 256, &vsz);

    printf("key: %s, value: %s\n", "key1", value);

    item_delete("key1", 4);
    int r = item_get("key1", 4, value, 256, &vsz);
    assert(r == 1);

    char buf[1024];

    print_stat(buf, sizeof(buf));

    printf("%s", buf);

    lru_free();
    print_stat(buf, sizeof(buf));
    printf("%s", buf);

    return 0;
}
