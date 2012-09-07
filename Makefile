CC = gcc
CFLAGS = -W -Wall -Werror -g -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls 
DEBUG_CFLAGS = -fprofile-arcs -ftest-coverage
DEBUG_LDFLAGS = -lgcov

LIB_OBJS = \
	./hash.o\
	./lru.o

TESTAPP_OBJS = \
	./test/test.o

BENCH_OBJS = \
	./test/bench.o

LIBRARY = liblru.so

TESTAPP = testapp

BENCH = bench

all: $(LIBRARY) $(TESTAPP) $(BENCH)

$(LIBRARY): $(LIB_OBJS)
	$(CC) -fPIC -shared $^ -o $@

$(LIB_OBJS): %.o: %.c
	$(CC) -fPIC $(CFLAGS) $^ -c -o $@

$(TESTAPP): $(LIBRARY) $(TESTAPP_OBJS)
	$(CC) -L. $^ -o $@ -llru

$(TESTAPP_OBJS): %.o: %.c
	$(CC) $(CFLAGS) -I. $^ -c -o $@

$(BENCH): $(LIBRARY) $(BENCH_OBJS)
	$(CC) -L. $^ -o $@ -llru

$(BENCH_OBJS): %.o: %.c
	$(CC) $(CFLAGS) -I. $^ -c -o $@

test: $(TESTAPP) $(BENCH)
	./$(TESTAPP)
	./$(BENCH)

clean:
	-rm -f $(LIB_OBJS)	$(TEST_OBJS)
	-rm -f $(LIBRARY)  $(TEST)
