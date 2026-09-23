TEST_SRCS := main.c
TEST_OBJS := $(patsubst %.c,%.o,$(TEST_SRCS))
TEST_DEPS := $(patsubst %.c,%.d,$(TEST_SRCS))

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
	rm -f test

-include $(TEST_DEPS)
