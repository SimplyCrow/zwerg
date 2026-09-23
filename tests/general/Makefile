TEST_SRCS := main.c
TEST_OBJS := $(patsubst %.c,%.o,$(TEST_SRCS))

test: $(TEST_OBJS) $(DWARF_LIB_ARCHIVE)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: run
run: test
	./test

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f *.o
	rm test
