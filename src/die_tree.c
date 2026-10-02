#include "die_tree.h"

#include <stdio.h>
#include <stdarg.h>

#include "encoding.h"
#include "abbrev.h"
#include "string_tables.h"
#include "form_parser.h"

static inline void contrib_debug_lenter(struct parse_contrib_t *contrib)
{
        ++contrib->current_level;
}

static inline void contrib_debug_lleave(struct parse_contrib_t *contrib)
{
        if(contrib->current_level != 0)
                --contrib->current_level;
}

static void parse_die(struct parse_contrib_t *contrib);
static void parse_die_children(struct parse_contrib_t *contrib);
static void parse_die_attribute(struct parse_contrib_t *contrib, struct abbrev_attr_t attribute);

static void parse_die_attribute(struct parse_contrib_t *contrib, struct abbrev_attr_t attribute)
{
        contrib_debug_lprintf(contrib, "        [%s:%s]\n"
                        , get_str_attribute_encoding(attribute.name)
                        , get_str_attribute_form_encoding(attribute.form));

        if(attribute.form == DW_FORM_indirect) {
                attribute.form = bstream_uleb128(&contrib->sdies);
                if(attribute.form == DW_FORM_indirect) {
                        bstream_trap(&contrib->sdies, "Indirect form attribute linked to an indirect form (cycle)");
                }
                printf("        -> indirect form: %s\n", get_str_attribute_form_encoding(attribute.form));
                parse_die_attribute(contrib, attribute);
        }

        form_parser_t form_parser = get_form_parser(attribute.form);
        if(!form_parser) {
                bstream_trap(&contrib->sdies, "Did not find form parser for form %s (0x%02lx)"
                                , get_str_attribute_form_encoding(attribute.form)
                                , attribute.form);
        }

        form_parser(contrib, attribute);
}

static void parse_die_children(struct parse_contrib_t *contrib)
{
        contrib_debug_lenter(contrib);
        if(bstream_peek_u8(&contrib->sdies) == 0) {
                bstream_trap(&contrib->sdies, "Expected children but failed");
        }

        while(bstream_peek_u8(&contrib->sdies)) {
                parse_die(contrib);
        }
        bstream_u8(&contrib->sdies); // skip children termination byte

        contrib_debug_lleave(contrib);
}

static void parse_die(struct parse_contrib_t *contrib)
{
        did_t abbrev_index = bstream_uleb128(&contrib->sdies);
        struct abbrev_entry_t *abbrev = abbrev_table_get(contrib->abbrev_table, abbrev_index);
        if(!abbrev) {
                bstream_trap(&contrib->sdies, "Did not find abbrev at index %zu\n", (size_t)abbrev_index);
        }

        contrib_debug_lprintf(contrib, "* tag {0x%02lx %s attr_count=%zu} %s\n"
                        , abbrev_index
                        , get_str_tag_encoding(abbrev->tag)
                        , abbrev->attr_count
                        , (abbrev->children) ? "with children" : "no children");

        for(size_t i = 0; i < abbrev->attr_count; ++i) {
                struct abbrev_attr_t attribute = abbrev->attrs[i];
                parse_die_attribute(contrib, attribute);
        }

        if(!abbrev->children) {
                return;
        }

        parse_die_children(contrib);
}

bool die_tree_parse(struct die_tree_t *tree, struct dwarf_sections_t sections)
{
        struct parse_t parse = {0};
        parse.sections = sections;

        const size_t info_size = sections.info.size;
        bstream_init(&parse.sdies, sections.info.data, info_size);
        int error = setjmp(parse.sdies.err_return);
        if(error) {
                fprintf(stderr, "\n[ERROR] [AT 0x%lx (%zu)] [LEN 0x%lx (%zu)] Could not parse contribution (global): %s\n"
                                , parse.sdies.pos, parse.sdies.pos
                                , parse.sdies.length, parse.sdies.length
                                , parse.sdies.err_message ? parse.sdies.err_message : "NO ERROR MESSAGE");
                return false;
        }

        while(bstream_left(&parse.sdies)) {
                struct parse_contrib_t contrib = {0};
                contrib.parse = &parse;

                uint64_t contrib_size = bstream_u32(&parse.sdies);
                contrib.dwarf64 = (contrib_size >= DWARF_DWARF64_IDENTIFIER);
                if(contrib.dwarf64) {
                        contrib_size = bstream_u64(&parse.sdies);
                }

                bstream_init_sub(&contrib.sdies, &parse.sdies, contrib_size);
                int error = setjmp(contrib.sdies.err_return);
                if(error) {
                        fprintf(stderr, "\n[ERROR] [AT 0x%lx (%zu)] [LEN 0x%lx (%zu)] Could not parse contribution (sub): %s\n"
                                        , contrib.sdies.pos, contrib.sdies.pos
                                        , contrib.sdies.length, contrib.sdies.length
                                        , contrib.sdies.err_message ? contrib.sdies.err_message : "NO ERROR MESSAGE");
                        bstream_trap(&parse.sdies, "Error while parsing contribution");
                }

                const uint16_t version       = bstream_u16(&contrib.sdies);
                const uint8_t  unit_type     = bstream_u8(&contrib.sdies);
                const uint8_t  address_size  = bstream_u8(&contrib.sdies);
                const doff_t   abbrev_offset = parse_contrib_dwarf_offset(&contrib);

                printf(" == Unit Header ==\n");
                printf("Length:       0x%lx (%ld)\n", contrib_size, contrib_size);
                printf("Version:      0x%x (%d)\n", version, version);
                printf("Unit type:    0x%x (%d)\n", unit_type, unit_type);
                printf("Address size: 0x%x (%d)\n", address_size, address_size);
                printf("debug abbrev offset: 0x%lx (%ld)\n", abbrev_offset, abbrev_offset);
                printf("=====\n");

                contrib.address_size = address_size;

                if(unit_type != DW_UT_compile) {
                        bstream_trap(&parse.sdies, "Currently only compile unit headers are supported\n");
                }

                contrib.abbrev_table = abbrev_table_create(sections.abbrev, abbrev_offset);
                if(!contrib.abbrev_table) {
                        bstream_trap(&parse.sdies, "Could not create abbrev table");
                }

                parse_die(&contrib);

                if(bstream_left(&contrib.sdies)) {
                        bstream_trap(&contrib.sdies, "%zu undefined bytes left", bstream_left(&contrib.sdies));
                }
        }

        return true;
}
