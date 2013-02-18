CC = gcc
CFLAGS = -W -Wall -Werror -g -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls 
DEBUG_CFLAGS = -fprofile-arcs -ftest-coverage
DEBUG_LDFLAGS = -lgcov

LIB_OBJS = \
	./hash.o\
	./lru.o

LIB_JNI_OBJS = \
	./binding/lru_jni.o

TESTAPP_OBJS = \
	./test/test.o

BENCH_OBJS = \
	./test/bench.o

LIBRARY = liblru.so

LIBRARY_JNI = liblrujni.so

TESTAPP = testapp

BENCH = bench

all: $(LIBRARY) $(LIBRARY_JNI) $(TESTAPP) $(BENCH)

.PHONY: all

$(LIBRARY): $(LIB_OBJS)
	$(CC) -fPIC -shared $^ -o $@

$(LIB_OBJS): %.o: %.c
	$(CC) -fPIC $(CFLAGS) $^ -c -o $@

$(TESTAPP): $(LIB_OBJS) $(TESTAPP_OBJS)
	$(CC) $^ -o $@ 

$(TESTAPP_OBJS): %.o: %.c
	$(CC) $(CFLAGS) -I. $^ -c -o $@

$(BENCH): $(LIB_OBJS) $(BENCH_OBJS)
	$(CC) $^ -o $@ 

$(BENCH_OBJS): %.o: %.c
	$(CC) $(CFLAGS) -I. $^ -c -o $@

jni: $(LIBRARY_JNI)

$(LIBRARY_JNI): $(LIB_OBJS) $(LIB_JNI_OBJS)
	$(CC) -fPIC -shared $^ -o $@

$(LIB_JNI_OBJS): %.o: %.c
	$(CC) -fPIC $(CFLAGS) -I. $^ -c -o $@

test: $(TESTAPP) $(BENCH)
	./$(TESTAPP)
	./$(BENCH)

clean:
	-rm -f $(LIB_OBJS) $(TESTAPP_OBJS) $(BENCH_OBJS) $(LIB_JNI_OBJS)
	-rm -f $(LIBRARY) $(LIBRARY_JNI) $(TESTAPP) $(BENCH)

.PHONY: clean
