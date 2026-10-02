// SPDX-License-Identifier: GPL-3.0-or-later
/*
 *  dwarf library
 *
 *  Copyright (C) 2026 Jonathan Kowalski <jonathan.kowalski2306@gmail.com>
 */

#include <libdwarf/dwarf.h>

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>
#include <assert.h>

#include "leb128.h"
#include "abbrev.h"
#include "encoding.h"
#include "string_tables.h"
#include "bstream.h"
#include "die_tree.h"

#if 1
        #define TTY_COLOR_RED   "\033[1;31m"
        #define TTY_COLOR_GREEN "\033[1;32m"
        #define TTY_COLOR_RESET "\033[0m"
#else
        #define TTY_COLOR_RED   ""
        #define TTY_COLOR_GREEN ""
        #define TTY_COLOR_RESET ""
#endif

void hex_dump(const void* _data, size_t size)
{
        const uint8_t *data = _data;

        size_t line = 0;
        for (size_t i = 0; i < size; ++i) {
                if(i % 16 == 0) {
                        printf("0x%06x: ", (uint32_t)i);
                }

                const uint8_t c = data[i];

                if(isprint(c) && isatty(1))  printf(TTY_COLOR_GREEN);
                else if(c != 0 && isatty(1)) printf(TTY_COLOR_RED);
                printf("%02x", (int)c);
                if(isatty(1)) {
                        printf(TTY_COLOR_RESET);
                }
                if(i % 2 == 1) printf(" ");

                if(i + 1 == size) {
                        size_t bytes_left = 16 * (line + 1) - i - 1;
                        size_t spaces = bytes_left * 2 + bytes_left / 2;
                        for(size_t i = 0; i < spaces; ++i) {
                                printf(" ");
                        }
                }

                if(i % 16 == 15 || i + 1 == size) {
                        printf(" | ");
                        for(size_t j = 0; j < 16; ++j) {
                                size_t i = 16 * line + j;
                                if(i >= size) {
                                        printf(" ");
                                        continue;
                                }

                                const uint8_t c = data[i];
                                if(isprint(c))  {
                                        if(isatty(1)) {
                                                printf(TTY_COLOR_GREEN);
                                        }
                                        printf("%c", c);
                                } else if(c != 0) {
                                        if(isatty(1)) {
                                                printf(TTY_COLOR_RED);
                                        }
                                        printf(".");
                                } else  {
                                        printf(".");
                                }
                                if(isatty(1)) {
                                        printf(TTY_COLOR_RESET);
                                }
                        }
                        if(isatty(1)) {
                                printf(TTY_COLOR_RESET);
                        }
                        printf("\n");
                        ++line;
                }
        }
}

typedef uint32_t off32_t;
typedef uint64_t off64_t;

struct unit_header32_t {
        uint32_t initial_length;
        uint16_t version;
        uint8_t  unit_type;
        uint8_t  address_size;
        off32_t  debug_abbrev_offset;
} __attribute__((packed));

struct unit_header64_t {
        uint32_t format_identifier;
        uint64_t initial_length;
        uint16_t version;
        uint8_t  unit_type;
        uint8_t  address_size;
        off64_t  debug_abbrev_offset;
} __attribute__((packed));

struct unit_header_t {
        union {
                struct unit_header32_t f32;
                struct unit_header64_t f64;
        };
} __attribute__((packed));

struct dwarf_context_t {
        struct dwarf_sections_t sections;
        struct abbrev_table_t  *abbrev_table;

        size_t  current_level;
        uint8_t offset_size;
        uint8_t address_size;

        // parsing
        struct bstream_t sdies;
};

struct dwarf_context_t *dwarf_init_context(struct dwarf_sections_t sections)
{
        printf("Initializing dwarf context\n");

        struct die_tree_t tree;
        if(!die_tree_parse(&tree, sections)) {
                printf("Could not parse tree\n");
                return NULL;
        }

        printf("Initialized dwarf context\n");
        return (void*)1;
}
