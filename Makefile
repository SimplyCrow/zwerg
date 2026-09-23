export CC := gcc
export CFLAGS := -Wall -Wextra -MMD -MP -g -I$(realpath ./include)

export DWARF_LIB_ARCHIVE = $(shell realpath libdwarf.a)

LIB_SRCS := dwarf.c abbrev.c string_tables.c encoding.c
LIB_OBJS := $(patsubst %.c,./src/%.o,$(LIB_SRCS))
LIB_DEPS := $(patsubst %.c,./src/%.d,$(LIB_SRCS))

libdwarf.a: $(LIB_OBJS)
	ar -rs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: tests run-tests
tests: libdwarf.a
	$(MAKE) --no-print-directory -C ./tests/general
run-tests: tests libdwarf.a
	$(MAKE) --no-print-directory -C ./tests/general run

.PHONY: clean
clean:
	rm -f ./src/*.o
	rm $(DWARF_LIB_ARCHIVE)
	$(MAKE) --no-print-directory -C ./tests/general clean

-include $(LIB_DEPS)
