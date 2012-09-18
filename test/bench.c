#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

#include "lru.h"

static struct config {
    size_t maxbytes;
    int num;
    int keysize;
    int datasize;
    int hashpower;
}config;

static void usage(void) {
    printf("-n <num>    total number of loop(default 1000,000)\n"
           "-m <num>    max memory for userd in megabytes(default 512MB)\n"
           "-k <num>    key size(default 16B)\n"
           "-d <num>    data size(default 32B)\n"
           "-p <num>    hash power level(default 16, 64K)\n"
           "-h          print this help and exit\n");
}

#define KEY_RAN_LEN 1024 * 1024 
static char *key_ = NULL;
static int pos_ = 0;
static int i_ = 0;
static int num_ = 0;
static char *fmt_;

static void generate_key_init() {
    char nbuf[64];
    num_ = snprintf(nbuf, sizeof(nbuf), "%d", config.num);

    snprintf(nbuf, sizeof(nbuf), "%%.*s%%0%dd",num_);
    fmt_ = strdup(nbuf);

    key_ = malloc(KEY_RAN_LEN);
    srandom(time(NULL));
    int i = 0;
    for ( i = 0; i < KEY_RAN_LEN; i++) {
        key_[i] = (char) (random() % 58) + 65; 
    }
}

static char* generate_key(int len) {
    if (pos_ + len > KEY_RAN_LEN) {
        pos_ = (++i_) % KEY_RAN_LEN;
    }
    pos_ += len;
    return key_ + pos_ - len;
}

static inline void generate_key_reset(void) {
    pos_ = 0;
    i_ = 0;
}

static void print_env(void) {
    printf("-----------------------------------------\n");
    printf("PID:         %d\n", (int)getpid());
    printf("ItemSize:    %zd\n", ITEM_size);
    printf("Keys:        %d bytes each\n", config.keysize);
    printf("Values:      %d bytes each\n", config.datasize);
    printf("Entries:     %d\n", config.num);

    printf("DataSize:    %.1f MB\n", (1L *(config.keysize + config.datasize) * config.num ) / 1048576.0);
    printf("TotalSize:   %.1f MB\n", (1L *(config.keysize + config.datasize + ITEM_size) * config.num ) / 1048576.0);
    printf("CacheSize:   %.1f MB\n", config.maxbytes / 1048576.0);
    printf("-----------------------------------------\n");
}

static long long start_;
static int done_, next_report_;
static char *benchmark_;

static long long mstime(void) {
    struct timeval tv;
    long long mst;

    gettimeofday(&tv, NULL);
    mst = ((long)tv.tv_sec)*1000;
    mst += tv.tv_usec/1000;
    return mst;
}


static void bench_start(char *bench) {
    start_ = mstime();
    benchmark_ = bench;
    done_ = 0;
    next_report_ = 0;
}

static void process_report(void) {
    done_++;
    if (done_ >= next_report_) {
        if (next_report_ < 1000) {
            next_report_ += 100;
        } else if (next_report_ < 5000) {
            next_report_ += 500;
        } else if (next_report_ < 10000) {
            next_report_ += 1000;
        } else if (next_report_ < 50000) {
            next_report_ += 5000;
        } else if (next_report_ < 100000) {
            next_report_ += 10000;
        } else if (next_report_ < 500000) {
            next_report_ += 50000;
        } else {
            next_report_ += 100000;
        }
        printf("... %s finished %d ops%30s\r", benchmark_, done_, "");
        fflush(stdout);
    }
}

static void bench_stop(void) {
    long long stop_ = mstime();
    printf("%-12s : take %lld ms, %11.3f op/s\n", benchmark_, (stop_ - start_),
         config.num * 1000.0 / (stop_ - start_));
}

static void config_init(void) {
    config.num = 1000000;
    config.maxbytes = 512 * 1024 * 1024;
    config.keysize = 16;
    config.datasize = 32;
    config.hashpower = 16;
}

static void print_stat(lru *l) {
    char bstat[1024];
    stat_print(l, bstat, sizeof(bstat));
    printf("-----------------------------------------\n");
    printf("STAT\n%s", bstat);
    printf("-----------------------------------------\n");
}

int main(int argc, char **argv) {
    config_init();

    int c;
    
    while (-1 != (c = getopt(argc, argv, "n:m:k:d:p:h"))) {
        switch(c) {
            case 'n':
                config.num = atoi(optarg);
                break;
            case 'm':
                config.maxbytes = ((size_t)atoi(optarg)) * 1024 * 1024;
                break;
            case 'k':
                config.keysize = atoi(optarg);
                break;
            case 'd':
                config.datasize = atoi(optarg);
                break;
            case 'p':
                config.hashpower = atoi(optarg);
                break;
            case 'h':
                usage();
                return EXIT_SUCCESS;
            default:
                usage();
                return EXIT_FAILURE;
        }
    }

    generate_key_init();
    print_env();
    lru *l = lru_init(config.maxbytes, config.hashpower);

    char *bvalue = malloc(config.datasize);
    memset(bvalue, 'x', config.datasize);

    char *key = malloc(config.keysize);
    memset(key, 0, config.keysize);

    int gnum = config.keysize - num_;

    generate_key_reset();
    bench_start("SET");
    int i;
    for (i = 0; i < config.num; i++) {
        snprintf(key, config.keysize, fmt_, gnum, generate_key(gnum), i);
        int r = item_set(l, key, config.keysize, bvalue, config.datasize);
        assert(r == 0);
        process_report();
    }
    bench_stop();
    print_stat(l);

    char *buf = malloc(config.datasize);
    size_t sz;
    generate_key_reset();
    bench_start("GET");
    for (i = 0; i < config.num; i++) {
        snprintf(key, config.keysize, fmt_, gnum, generate_key(gnum), i);
        int r = item_get(l, key, config.keysize, buf, config.datasize, &sz);
        if (!r) {
            assert((int)sz == config.datasize);
            assert(memcmp(bvalue, buf, config.datasize) == 0);
        }
        memset(buf, 0, config.datasize);
        process_report();
    }
    bench_stop();
    print_stat(l);

    generate_key_reset();
    bench_start("DELETE");
    for (i = 0; i < config.num; i++) {
        snprintf(key, config.keysize, fmt_, gnum, generate_key(gnum), i);
        item_delete(l, key, config.keysize);
        process_report();
    }
    bench_stop();
    print_stat(l);
    
    free(buf);
    free(bvalue);
    free(key);
    free(fmt_);
    free(key_);
   
    lru_free(l);
    /*
    printf("print any key to exit...\n");
    getchar();
    */
    return EXIT_SUCCESS;
}
