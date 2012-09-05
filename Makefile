CC = gcc
DEBUG =	-g -DERROR
CFLAGS += -W -Wall -Werror $(DEBUG)

LIB_OBJS = \
	./hash.o\
	./lru.o

TEST_OBJS = \
	./test/test.o

LIBRARY = liblru.so

TEST = testapp

all: $(LIBRARY) $(LIBRARY_S) $(TEST)

$(LIBRARY): $(LIB_OBJS)
	$(CC) -fPIC -shared $^ -o $@

$(LIB_OBJS): %.o: %.c
	$(CC) -fPIC $(CFLAGS) $^ -c -o $@

$(TEST): $(TEST_OBJS)
	$(CC) -L. $^ -o $@ -llru

$(TEST_OBJS): %.o: %.c
	$(CC) $(CFLAGS) -I. $^ -c -o $@

clean:
	-rm -f $(LIB_OBJS)	$(TEST_OBJS)
	-rm -f $(LIBRARY)  $(TEST)
