#include <libdwarf/dwarf.h>

#include <stdio.h>
#include <ctype.h>

#include "leb128.h"
#include "abbrev.h"

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

                if(isprint(c))  printf(TTY_COLOR_GREEN);
                else if(c != 0) printf(TTY_COLOR_RED);
                printf("%02x", (int)c);
                printf(TTY_COLOR_RESET);
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
                                if(isprint(c))  printf(TTY_COLOR_GREEN"%c", c);
                                else if(c != 0) printf(TTY_COLOR_RED".");
                                else            printf(".");
                                printf(TTY_COLOR_RESET);
                        }
                        printf(TTY_COLOR_RESET"\n");
                        ++line;
                }
        }
}

struct dwarf_context_t {
        struct dwarf_sections_t sections;
        struct abbrev_table_t abbrev_table;
};

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


struct dwarf_context_t *dwarf_init_context(struct dwarf_sections_t sections)
{
        printf("Initializing dwarf context\n");

        struct unit_header_t *gen_unit_header = (void*)sections.info.data;
        if(gen_unit_header->f64.format_identifier >= 0xfffffff0) {
                fprintf(stderr, "DWARF 64 is currently unsupported\n");
                return NULL;
        }

        struct unit_header32_t *unit_header = &gen_unit_header->f32;

        printf(" == Unit Header (0x%02zu) ==\n", sizeof(struct unit_header32_t));
        printf("Length:       %d\n", unit_header->initial_length);
        printf("Version:      %d\n", unit_header->version);
        printf("Unit type:    %d\n", unit_header->unit_type);
        printf("Address size: %d\n", unit_header->address_size);
        printf("debug abbrev offset: %d\n", unit_header->debug_abbrev_offset);
        printf("=====\n");

        struct abbrev_table_t abbrev_table0 = {0};
        if(!abbrev_table_create(&abbrev_table0, sections.abbrev, unit_header->debug_abbrev_offset)) {
                fprintf(stderr, "Could not create abbrev table\n");
                return NULL;
        }

        printf("Abbreviation table:\n");

        uint8_t *abbrev_table = sections.abbrev.data;
        size_t   abbrev_off = 0;
        while(abbrev_table[abbrev_off] != 0) {
                uint64_t code, tag;
                abbrev_off += uleb128_decode(abbrev_table + abbrev_off, &code);
                abbrev_off += uleb128_decode(abbrev_table + abbrev_off, &tag);

                uint8_t children = abbrev_table[abbrev_off++];

                printf(" * [0x%02lx]: tag: 0x%02lx %s:\n", code, tag,
                                (children == 0x00) ? "no children" : "children");
                while(1) {
                        uint64_t name, form;
                        abbrev_off += uleb128_decode(abbrev_table + abbrev_off, &name);
                        abbrev_off += uleb128_decode(abbrev_table + abbrev_off, &form);
                        if(name == 0x00 && form == 0x00) {
                                break;
                        }
                        printf("     - 0x%02lx:0x%02lx\n", name, form);
                }
        }

        uint8_t *entries_start = (uint8_t*)(sections.info.data + sizeof(struct unit_header32_t));
        uint8_t *abbrev_start = (uint8_t*)(sections.abbrev.data);

        size_t   used_bytes = 0;
        uint64_t tag_encoding = 0x00;
        used_bytes += uleb128_decode(entries_start, &tag_encoding);

        printf("Abbrv offset: 0x%02lx (%zu)\n", tag_encoding, used_bytes);

        size_t   used_bytes_abbrev = 0;
        uint64_t abbrev_index = 0x00;
        used_bytes_abbrev += uleb128_decode(abbrev_start + used_bytes_abbrev + tag_encoding, &abbrev_index);
        printf("Stored: 0x%02lx\n", abbrev_index);


        return NULL;
}
