#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

static struct config {
    size_t maxbytes;
    int num;
    int keysize;
    int datasize;
}config;

static void usage(void) {
    printf("-n <num>    total number of loop(default 1000,000)\n"
           "-m <num>    max memory for userd in megabytes(default 64MB)\n"
           "-k <num>    key size\n"
           "-d <num>    data size\n"
           "-h          print this help and exit\n");
}

int main(int argc, char **argv) {
    int c;

    while (-1 != (c = getopt(argc, argv, "n:m:k:d:h:"))) {
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
            case 'h':
                usage();
                return EXIT_SUCCESS;
            default:
                usage();
                return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
