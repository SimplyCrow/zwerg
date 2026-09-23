#ifndef LIBDWARF_ABBREV_H
#define LIBDWARF_ABBREV_H

#include <libdwarf/dwarf.h>

#include <stdbool.h>

#include "leb128.h"

struct abbrev_attr_t {
        uint64_t name;
        uint64_t form;
        int64_t  implicit_const;
};

struct abbrev_entry_t {
        uint64_t index;
        uint64_t tag;
        bool     children;
        size_t   attr_count;
        struct abbrev_attr_t *attrs;
};

struct abbrev_table_t {
        size_t count;
        struct abbrev_entry_t *entries;
};

bool abbrev_table_create(
                struct abbrev_table_t *table,
                struct dwarf_buffer_t  abbrev_section,
                size_t offset
);

struct abbrev_entry_t *abbrev_table_get(
                struct abbrev_table_t *table,
                uint64_t index
);

#endif // LIBDWARF_ABBREV_H
