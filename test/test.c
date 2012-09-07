#include <stdio.h>
#include <assert.h>
#include "lru.h"

enum test_return { TEST_SKIP, TEST_PASS, TEST_FAIL };

typedef enum test_return (*test_func)(void);

struct testcase {
    const char *description;
    test_func function;
};

static enum test_return lru_normal_op(int new, size_t mb, char *key, size_t nkey, char *value, size_t nvalue) {
    char buf[64];
    size_t sz;

    if (new) {
        lru_init(mb);
    }

    int r = item_set(key, nkey, value, nvalue);

    if (r) {
        return TEST_FAIL;
    }

    r = item_get(key, nkey, buf, sizeof(buf), &sz);
    assert(r == 0);
    assert(sz == nvalue);
    assert(memcmp(value, buf, sz) == 0);

    r = item_delete(key, nkey);
    assert(r == 0);

    if (new) {
        lru_free();
        stat_reset();
    }
    return TEST_PASS;
}

static enum test_return lru_normal_test(void) {
    char *key = "key1";
    char *value = "value1";

    return lru_normal_op(1, 0, key, strlen(key), value, strlen(value) + 1);
}

static enum test_return lru_overwrite_test(void) {
    return TEST_SKIP;
}

static enum test_return lru_big_set_test(void) {
    char *key = "key1";
    char *value = "value1";

    int malloc = ITEM_size + strlen(key) + 1 + strlen(value) + 1; 

    enum test_return r = lru_normal_op(1, malloc, key, strlen(key), value, strlen(value) + 1);

    if ( r != TEST_PASS) {
        return r;
    }

    r = lru_normal_op(1, malloc - 1, key, strlen(key), value, strlen(value) + 1);

    if ( r == TEST_FAIL) {
        return TEST_PASS;
    } else {
        return TEST_FAIL;
    }
}

static enum test_return lru_evict_test(void) {
    char key[5][5] = {"key1", "key2", "key3", "key4", "key5"};
    char *value = "value1";

    size_t nkey = 4;
    size_t nvalue = strlen(value);

    int malloc = ITEM_size + nkey + nvalue + 1; 
    int r, i = 0, loop;

    loop = sizeof(key) / (nkey + 1);

    lru_init(malloc * loop);

    size_t sz;
    char buf[256];


    for (i = 0; i < loop; i++) {
        r = item_set(key[i], nkey, value, nvalue);
        assert(r == 0);
    }

    for (i = 0; i < loop; i++) {
        r = item_get(key[i], nkey, buf, sizeof(buf), &sz);
        assert(r == 0);
        assert(sz == nvalue);
        assert(memcmp(value, buf, sz) == 0);
        memset(buf, 0, sizeof(buf));
    }

    //evict 3
    char *key2 = "key6";
    size_t nvalue2 = malloc * 2 + 2;
    char value2[nvalue2];

    memset(value2, (int)'0', nvalue2);

    value2[nvalue2 -1] = '\0';
    value2[0] = 'a';

    r = item_set(key2, nkey, value2, nvalue2);
    assert(r == 0);

    for (i = 0; i < 3; i++) {
        r = item_get(key[i], nkey, buf, sizeof(buf), &sz);
        assert(r == 1);
    }
    for (i = 3; i < loop; i++) {
        r = item_get(key[i], nkey, buf, sizeof(buf), &sz);
        assert(r == 0);
        assert(sz == nvalue);
        assert(memcmp(value, buf, sz) == 0);
        memset(buf, 0, sizeof(buf));
    }

    r = item_get(key2, nkey, buf, sizeof(buf), &sz);
    assert(r == 0);
    assert(sz == nvalue2);
    assert(memcmp(value2, buf, sz) == 0);
    memset(buf, 0, sizeof(buf));
    
    return TEST_PASS;
}

static enum test_return lru_stat_test(void) {
    return TEST_SKIP;
}

struct testcase testcases[] = {
    { "normal_test", lru_normal_test },
    { "overwrite_test", lru_overwrite_test },
    { "big_test", lru_big_set_test },
    { "evict_test", lru_evict_test },
    { "stat_test", lru_stat_test },
    { NULL, NULL }
};

int main(void)
{
    int exitcode = 0;
    int ii = 0, num_cases = 0;

    for (num_cases = 0; testcases[num_cases].description; num_cases++);

    printf("1..%d\n", num_cases);

    for (ii = 0; testcases[ii].description != NULL; ++ii) {
        fflush(stdout);
        enum test_return ret = testcases[ii].function();
        if (ret == TEST_SKIP) {
            fprintf(stdout, "ok # SKIP %d - %s\n", ii + 1, testcases[ii].description);
        } else if (ret == TEST_PASS) {
            fprintf(stdout, "ok %d - %s\n", ii + 1, testcases[ii].description);
        } else {
            fprintf(stdout, "not ok %d - %s\n", ii + 1, testcases[ii].description);
            exitcode = 1;
        }
        fflush(stdout);
    }

    return exitcode;
}
