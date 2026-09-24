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
        struct unit_header32_t  header;
        struct abbrev_table_t   abbrev_table;

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
        switch(context->header.address_size) {
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
                bstream_trap(&context->sdies, "Tired to get string but no .debug_str section present");
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
        printf("        - attr: %s (0x%lx):%s "
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
        if(bstream_peek_u8(&context->sdies) == 0) {
                bstream_trap(&context->sdies, "Expected children but failed");
        }

        while(bstream_peek_u8(&context->sdies) != 0) {
                parse_die(context);
        }
}

static void parse_die(struct dwarf_context_t *context)
{

        uint64_t abbrev_index = bstream_uleb128(&context->sdies);
        struct abbrev_entry_t *abbrev = abbrev_table_get(&context->abbrev_table, abbrev_index);
        if(abbrev == NULL) {
                fprintf(stderr, "Did not find abbrev at index %zu\n", (size_t)abbrev_index);
                bstream_trap(&context->sdies, "Did not find abbrev");
        }

        printf(" * tag %s attr_count: %zu %s\n",
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

        printf("        - children:\n");
        parse_die_children(context);
}

static bool parse(struct dwarf_context_t *context)
{
        struct dwarf_buffer_t info = context->sections.info;
        bstream_init(&context->sdies
                        , info.data + sizeof(struct unit_header32_t)
                        , info.size - sizeof(struct unit_header32_t));

        int error = setjmp(context->sdies.err_return);
        if(error != 0) {
                fprintf(stderr, "[ERROR] [AT 0x%lx (%zu)] Could not parse dies %d: %s\n"
                                , context->sdies.pos, context->sdies.pos,
                                error,
                                context->sdies.err_message ? context->sdies.err_message : "NO ERROR MESSAGE");
                return false;
        }

        parse_die_children(context);

        return true;
}

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

        struct abbrev_table_t abbrev_table = {0};
        if(!abbrev_table_create(&abbrev_table, sections.abbrev, unit_header->debug_abbrev_offset)) {
                fprintf(stderr, "Could not create abbrev table\n");
                return NULL;
        }

        struct dwarf_context_t context = {0};
        context.sections     = sections;
        context.header       = *unit_header;
        context.abbrev_table = abbrev_table;

        parse(&context);

        return NULL;
}
