#ifndef LIBDWARF_DWARF_H
#define LIBDWARF_DWARF_H

#include <stdint.h>
#include <stddef.h>

struct dwarf_buffer_t {
        uint8_t *data;
        size_t   size;
};

struct dwarf_sections_t {
        struct dwarf_buffer_t info;
        struct dwarf_buffer_t abbrev;
        struct dwarf_buffer_t line;
        struct dwarf_buffer_t line_str;
        struct dwarf_buffer_t loclists;
        struct dwarf_buffer_t rnglists;
        struct dwarf_buffer_t str;
        struct dwarf_buffer_t str_offsets;
        struct dwarf_buffer_t names;
        struct dwarf_buffer_t macro;
        struct dwarf_buffer_t frame;
};

struct dwarf_context_t;
struct dwarf_context_t *dwarf_init_context(struct dwarf_sections_t sections);

#endif // LIBDWARF_DWARF_H
