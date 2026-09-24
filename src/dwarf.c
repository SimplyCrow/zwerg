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
        struct abbrev_table_t   abbrev_table;

        size_t  current_level;
        uint8_t offset_size;
        uint8_t address_size;

        // parsing
        struct bstream_t sdies;
};

static size_t context_dwarf_offset(struct dwarf_context_t *context)
{
        // TODO: DWARF 64-bit
        return (size_t)bstream_u32(&context->sdies);
}

static uint64_t context_arch_address(struct dwarf_context_t *context)
{
        // TODO: more address sizes
        switch(context->address_size) {
                case 4:
                        return bstream_u32(&context->sdies);
                case 8:
                        return bstream_u64(&context->sdies);
                default:
                        bstream_trap(&context->sdies, "Undefined architecture address size");
        }
        return 0;
}

static const char *context_get_string(struct dwarf_context_t *context, size_t offset)
{
        if(context->sections.str.data == NULL || context->sections.str.size == 0) {
                bstream_trap(&context->sdies, "Tried to get string but no .debug_str section present");
        }

        const struct dwarf_buffer_t str = context->sections.str;
        if(offset >= str.size) {
                fprintf(stderr, "tried access: %zu\n", offset);
                bstream_trap(&context->sdies, "Tried to get string out of bounds");
        }

        return (const char *)(str.data + offset);
}

static void parse_attribute(struct dwarf_context_t *context, struct abbrev_attr_t attribute)
{
        for(size_t i = 0; i < context->current_level; ++i) {
                printf("	");
        }
        printf("        - [0x%05lx]: %s (0x%lx):%s "
                        , bstream_tell(&context->sdies)
                        , get_str_attribute_encoding(attribute.name)
                        , attribute.name
                        , get_str_attribute_form_encoding(attribute.form));

        if(attribute.form == DW_FORM_indirect) {
                        attribute.form = bstream_uleb128(&context->sdies);
                        if(attribute.form == DW_FORM_indirect) {
                                bstream_trap(&context->sdies, "Indirect form attribute linked to an indirect form (cycle)");
                        }
                        printf("ref to form: 0x%lx (indirect)\n", attribute.form);
                        parse_attribute(context, attribute);
                        return;
        }

        if(attribute.form == DW_FORM_addr) {
                const uint64_t address = context_arch_address(context);
                printf("%lx (address)\n", address);
                return;
        }

        if(attribute.form == DW_FORM_string) {
                const char *string = bstream_string(&context->sdies);
                printf("\"%s\" (string)\n", string);
                return;
        }

        if(attribute.form == DW_FORM_strp) {
                const size_t str_off = context_dwarf_offset(context);
                const char *string = context_get_string(context, str_off);
                printf("\"%s\" (strp)\n", string);
                return;
        }

        if(attribute.form == DW_FORM_strp) {
                const size_t str_off = context_dwarf_offset(context);
                const char *string = context_get_string(context, str_off);
                printf("\"%s\" (strp)\n", string);
                return;
        }

        const size_t attr_size = get_mapping_form_argument_size(attribute.form);
        if(attr_size == DW_ATT_FORM_ARG_SIZE_INVALID) {
                bstream_trap(&context->sdies, "Encountered invalid attribute size");
        }

        if(attr_size == DW_ATT_FORM_ARG_SIZE_ULEB128) {
                const uint64_t value = bstream_uleb128(&context->sdies);
                printf("0x%lx [%lu] (uleb128)\n", value, value);
                return;
        }

        if(attr_size == DW_ATT_FORM_ARG_SIZE_SLEB128) {
                const int64_t value = bstream_sleb128(&context->sdies);
                printf("0x%lx [%ld] (sleb128)\n", value, value);
                return;
        }

        if(attr_size == DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZEX
                        || attr_size == DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE1
                        || attr_size == DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE2
                        || attr_size == DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE4) {

                uint64_t buffer_size = 0;
                switch(attr_size) {
                        case DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZEX:
                                buffer_size = bstream_uleb128(&context->sdies);
                                break;
                        case DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE1:
                                buffer_size = bstream_u8(&context->sdies);
                                break;
                        case DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE2:
                                buffer_size = bstream_u16(&context->sdies);
                                break;
                        case DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE4:
                                buffer_size = bstream_u32(&context->sdies);
                                break;
                }

                bstream_advance(&context->sdies, buffer_size);
                printf("length: %zu (buffer)\n", (size_t)buffer_size);
                return;
        }

        printf(" size: %zu (raw data) ", attr_size);
        uint64_t value = 0;
        switch(attr_size) {
                case 1:
                        value = bstream_u8(&context->sdies);
                        break;
                case 2:
                        value = bstream_u16(&context->sdies);
                        break;
                case 4:
                        value = bstream_u32(&context->sdies);
                        break;
                case 8:
                        value = bstream_u64(&context->sdies);
                        break;
                default:
                        bstream_advance(&context->sdies, attr_size);
                        printf("value: (raw)\n");
                        return;

        }
        printf("value: %lx\n", value);
}

static void parse_die(struct dwarf_context_t *context);

static void parse_die_children(struct dwarf_context_t *context)
{
        for(size_t i = 0; i < context->current_level; ++i) {
                printf("	");
        }
        ++context->current_level;
        printf("-- enter children list [0x%05lx][%2zu] --\n", bstream_tell(&context->sdies), context->current_level);
        if(bstream_peek_u8(&context->sdies) == 0) {
                bstream_trap(&context->sdies, "Expected children but failed");
        }

        while(bstream_peek_u8(&context->sdies) != 0) {
                parse_die(context);
        }
        for(size_t i = 0; i < context->current_level; ++i) {
                printf("	");
        }
        printf("-- exit children list [0x%05lx][%2zu] --\n", bstream_tell(&context->sdies), context->current_level);
        bstream_u8(&context->sdies); // skip zero termination byte
        --context->current_level;
}

static void parse_die(struct dwarf_context_t *context)
{
        for(size_t i = 0; i < context->current_level; ++i) {
                printf("	");
        }
        printf("-- enter die [0x%05lx] --\n", bstream_tell(&context->sdies));
        const size_t start = context->sdies.data - context->sections.info.data
                + bstream_tell(&context->sdies);

        uint64_t abbrev_index = bstream_uleb128(&context->sdies);
        printf("abbrev_index = %lx\n", abbrev_index);
        struct abbrev_entry_t *abbrev = abbrev_table_get(&context->abbrev_table, abbrev_index);
        if(abbrev == NULL) {
                fprintf(stderr, "Did not find abbrev at index %zu\n", (size_t)abbrev_index);
                bstream_trap(&context->sdies, "Did not find abbrev");
        }

        for(size_t i = 0; i < context->current_level; ++i) {
                printf("	");
        }

        printf(" [0x%05lx][%2zu] * tag %s attr_count: %zu %s\n",
                        start,
                        context->current_level,
                        get_str_tag_encoding(abbrev->tag),
                        abbrev->attr_count,
                        (abbrev->children) ? "with children" : "no children");

        for(size_t i = 0; i < abbrev->attr_count; ++i) {
                struct abbrev_attr_t attribute = abbrev->attrs[i];
                parse_attribute(context, attribute);
                if(attribute.name == 0x90) {
                        fprintf(stderr, "invalid name\n");
                }
        }

        if(!abbrev->children) {
                return;
        }

        for(size_t i = 0; i < context->current_level; ++i) {
                printf("	");
        }

        printf("        - children:\n");
        parse_die_children(context);

        for(size_t i = 0; i < context->current_level; ++i) {
                printf("	");
        }
        printf("-- exit die [0x%05lx] --\n", bstream_tell(&context->sdies));
}

static bool parse_contribution(struct dwarf_context_t *context)
{
        int error = setjmp(context->sdies.err_return);
        if(error != 0) {
                fflush(stdout);
                fflush(stderr);
                fprintf(stderr, "\n[ERROR] [AT 0x%lx (%zu)] [LEN 0x%lx (%zu)] Could not parse contribution: %s\n"
                                , context->sdies.pos, context->sdies.pos
                                , context->sdies.length, context->sdies.length
                                , context->sdies.err_message ? context->sdies.err_message : "NO ERROR MESSAGE");
                return false;
        }

        parse_die(context);

        return true;
}

bool parse(struct dwarf_sections_t sections)
{
        struct dwarf_context_t context = {0};
        context.sections     = sections;

        const size_t info_size = sections.info.size;
        bstream_init(&context.sdies, sections.info.data, info_size);

        size_t info_size_used = 0;

        while((info_size_used < info_size) && (info_size - info_size_used)) {
                printf("DEB: 0x%lx\n", context.sdies.data - sections.info.data + context.sdies.pos);
                context.sdies.length = 4;
                const uint32_t initial_unit_length = bstream_u32(&context.sdies);
                if(initial_unit_length >= 0xfffffff0) {
                        fprintf(stderr, "DWARF 64 is currently unsupported\n");
                        return false;
                }

                context.sdies.length += initial_unit_length;

                const uint8_t offset_size = 4;

                const uint16_t version       = bstream_u16(&context.sdies);
                const uint8_t  unit_type     = bstream_u8(&context.sdies);
                const uint8_t  address_size  = bstream_u8(&context.sdies);
                const uint32_t abbrev_offset = bstream_u32(&context.sdies);

                context.current_level = 0;
                context.offset_size   = offset_size;
                context.address_size  = address_size;

                // TODO: respect initial length field in bstream to report
                // overruns

                printf(" == Unit Header AT 0x%lx==\n", info_size_used);
                printf("Length:       0x%x (%d)\n", initial_unit_length, initial_unit_length);
                printf("Version:      0x%x (%d)\n", version, version);
                printf("Unit type:    0x%x (%d)\n", unit_type, unit_type);
                printf("Address size: 0x%x (%d)\n", address_size, address_size);
                printf("debug abbrev offset: 0x%x (%d)\n", abbrev_offset, abbrev_offset);
                printf("=====\n");

                if(unit_type != DW_UT_compile) {
                        fprintf(stderr, "Currently only compile unit headers are supported\n");
                        return false;
                }

                if(!abbrev_table_create(&context.abbrev_table, sections.abbrev, abbrev_offset)) {
                        fprintf(stderr, "Could not create abbrev table\n");
                        return false;
                }

                const bool success = parse_contribution(&context);
                if(!success) {
                        fprintf(stderr, "Could not parse contribution\n");
                        return false;
                }

                info_size_used += context.sdies.length;
                const size_t padding = bstream_left(&context.sdies);
                printf("Ended unit with %zu padding\n", padding);
                bstream_advance(&context.sdies, padding);
                bstream_trim_front(&context.sdies);
        }

        return true;
}

struct dwarf_context_t *dwarf_init_context(struct dwarf_sections_t sections)
{
        printf("Initializing dwarf context\n");

        if(!parse(sections)) {
                return NULL;
        }

        printf("Initialized dwarf context\n");
        return (void*)1;
}
