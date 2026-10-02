// SPDX-License-Identifier: GPL-3.0-or-later
/*
 *  dwarf die tree
 *
 *  Copyright (C) 2026 Jonathan Kowalski <jonathan.kowalski2306@gmail.com>
 */

#ifndef LIBDWARF_DIE_TREE_H
#define LIBDWARF_DIE_TREE_H

#include <libdwarf/dwarf.h>

#include <stdio.h>
#include <stdarg.h>

#include "leb128.h"
#include "abbrev.h"
#include "bstream.h"

#define DWARF_MINIMAL_UNIT_HEADER_SIZE (17)
#define DWARF_DWARF64_IDENTIFIER (0xfffffff0)

struct parse_t {
        struct dwarf_sections_t sections;
        struct bstream_t sdies;
};

struct parse_contrib_t {
        const struct parse_t        *parse;
        const struct abbrev_table_t *abbrev_table;
        size_t  current_level;
        uint8_t address_size;
        bool    dwarf64;
        struct bstream_t sdies;
};

union attribute_value_t {
        struct dwarf_buffer_t buffer;
        const char *str;
        uleb128 uleb;
        sleb128 sleb;
        uint8_t   u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        uint64_t  u128[2];
};

struct attribute_t {
        did_t name;
        did_t form;
        union attribute_value_t value;
};

struct die_node_t {
        did_t tag;

        size_t attr_count;
        struct attribute_t attrs;

        size_t child_count;
        struct die_node_t *children;
};

struct die_tree_t {
        struct die_node_t *root;
};

static inline size_t parse_contrib_dwarf_offset(struct parse_contrib_t *contrib)
{
        return (contrib->dwarf64) ? bstream_u64(&contrib->sdies) : bstream_u32(&contrib->sdies);
}

static inline int contrib_debug_lprintf(struct parse_contrib_t *contrib, const char *fmt, ...)
{
        va_list params_format;
        va_start(params_format, fmt);
        printf("[0x%02lx] ", contrib->current_level);
        for(size_t i = 0; i < contrib->current_level; ++i)
                printf("\t");
        int return_value = vprintf(fmt, params_format);
        va_end(params_format);
        return return_value;
}

bool die_tree_parse(struct die_tree_t *tree, struct dwarf_sections_t sections);

#endif // LIBDWARF_DIE_TREE_H
