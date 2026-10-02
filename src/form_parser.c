#include "form_parser.h"

#include <stdlib.h>

struct form_parser_table_entry_t {
        did_t         form;
        form_parser_t parser;
};

static const struct form_parser_table_entry_t *parser_table_get();
static size_t parser_table_size();


static int compare_parsers(const void *_a, const void *_b)
{
        const struct form_parser_table_entry_t *a = (const struct form_parser_table_entry_t *)_a;
        const struct form_parser_table_entry_t *b = (const struct form_parser_table_entry_t *)_b;
        if(a->form < b->form)
                return -1;
        if(a->form > b->form)
                return 1;
        return 0;
}

form_parser_t get_form_parser(did_t form)
{
        struct form_parser_table_entry_t key = {0};
        key.form = form;

        const struct form_parser_table_entry_t *parser = bsearch(&key
                        , parser_table_get()
                        , parser_table_size()
                        , sizeof(struct form_parser_table_entry_t)
                        , compare_parsers);
        if(parser)
                return parser->parser;
        return NULL;
}

static inline const char *get_string(struct parse_contrib_t *contrib, doff_t offset)
{
        // TODO: Possible invalid string that goes out of bounds (missing termination)
        if(contrib->parse->sections.str.data == NULL || contrib->parse->sections.str.size == 0) {
                bstream_trap(&contrib->sdies, "Tried to get string but no .debug_str section is present");
        }

        const struct dwarf_buffer_t str_section = contrib->parse->sections.str;
        if(offset >= str_section.size) {
                bstream_trap(&contrib->sdies, "Tried to get string out of bounds. section size: %zu offset: %zu", str_section.size, offset);
        }

        return (const char *)(str_section.data + offset);
}

static inline const char *get_line_string(struct parse_contrib_t *contrib, doff_t offset)
{
        // TODO: Possible invalid string that goes out of bounds (missing termination)
        if(contrib->parse->sections.line_str.data == NULL || contrib->parse->sections.line_str.size == 0) {
                bstream_trap(&contrib->sdies, "Tried to get line string but no .line_str section is present");
        }

        const struct dwarf_buffer_t str_section = contrib->parse->sections.line_str;
        if(offset >= str_section.size) {
                bstream_trap(&contrib->sdies, "Tried to get line string out of bounds. section size: %zu offset: %zu", str_section.size, offset);
        }

        return (const char *)(str_section.data + offset);
}

static inline doff_t get_string_offset(struct parse_contrib_t *contrib, doff_t offset)
{
        if(contrib->parse->sections.str_offsets.data == NULL || contrib->parse->sections.str_offsets.size == 0) {
                bstream_trap(&contrib->sdies, "Tried to get string offset but no .debug_str_offsets section is present");
        }

        const struct dwarf_buffer_t offs_section = contrib->parse->sections.str_offsets;
        if((offset + sizeof(uint32_t)) > offs_section.size) {
                bstream_trap(&contrib->sdies, "Tried to get string offset out of bounds. section size: %zu offset: %zu", offs_section.size, offset);
        }

        if(contrib->dwarf64)
                return *(uint64_t*)(offs_section.data + offset);
        return *(uint32_t*)(offs_section.data + offset);
}

static inline darch_t arch_address(struct parse_contrib_t *contrib)
{
        switch(contrib->address_size) {
                case 4:
                        return bstream_u32(&contrib->sdies);
                case 8:
                        return bstream_u64(&contrib->sdies);
                default:
                        bstream_trap(&contrib->sdies, "Undefined architecture address size %zu", contrib->address_size);
        }
        return 0;
}

static void string(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const char *string = bstream_string(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> string: \"%s\"\n", string);
}

static void strp(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const doff_t str_off = parse_contrib_dwarf_offset(contrib);
        const char *string = get_string(contrib, str_off);
        contrib_debug_lprintf(contrib, "        -> string: \"%s\"\n", string);
}

static void line_strp(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const doff_t str_off = parse_contrib_dwarf_offset(contrib);
        const char *string = get_line_string(contrib, str_off);
        contrib_debug_lprintf(contrib, "        -> line string: \"%s\"\n", string);
}

static void addr(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const darch_t address = arch_address(contrib);
        contrib_debug_lprintf(contrib, "        -> address: 0x%08lx\n", address);
}

static void data1(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uint8_t data = bstream_u8(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> data: 0x%02x\n", data);
}

static void data2(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uint16_t data = bstream_u16(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> data: 0x%04x\n", data);
}

static void data4(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uint32_t data = bstream_u32(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> data: 0x%08x\n", data);
}

static void data8(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uint64_t data = bstream_u64(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> data: 0x%016lx\n", data);
}

static void data16(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uint64_t low_data  = bstream_u64(&contrib->sdies);
        const uint64_t high_data = bstream_u64(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> data: 0x%016lx%016lx\n", high_data, low_data);
}

static void sec_offset(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const doff_t offset = parse_contrib_dwarf_offset(contrib);
        contrib_debug_lprintf(contrib, "        -> sec offset: 0x%016lx\n", offset);
}

static void ref1(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uint8_t ref = bstream_u8(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> ref: 0x%02x\n", ref);
}

static void ref2(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uint16_t ref = bstream_u16(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> ref: 0x%04x\n", ref);
}

static void ref4(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uint32_t ref = bstream_u32(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> ref: 0x%08x\n", ref);
}

static void ref8(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uint64_t ref = bstream_u64(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> ref: 0x%016lx\n", ref);
}

static void ref_udata(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uleb128 ref = bstream_uleb128(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> ref: 0x%016lx\n", ref);
}

static void ref_addr(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const doff_t ref = parse_contrib_dwarf_offset(contrib);
        contrib_debug_lprintf(contrib, "        -> ref: 0x%016lx\n", ref);
}

static void ref_sig8(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const doff_t ref = bstream_u64(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> ref type signature: 0x%016lx\n", ref);
}

static void implicit_const(struct parse_contrib_t *contrib, struct abbrev_attr_t attribute)
{
        contrib_debug_lprintf(contrib, "        -> value: 0x%016lx\n", attribute.implicit_const);
}

static void flag(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uint8_t flag = bstream_u8(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> value: %s\n"
                        , (flag) ? "true" : "false");
}

static void flag_present(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        contrib_debug_lprintf(contrib, "        -> value: true\n");
}

static void exprloc(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uleb128 size = bstream_uleb128(&contrib->sdies);
        bstream_advance(&contrib->sdies, size);
        contrib_debug_lprintf(contrib, "        -> expression size: %ld\n", size);
}

static void loclistx(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uleb128 offset = bstream_uleb128(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> offset: %lx\n", offset);
}

static void rnglistx(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uleb128 offset = bstream_uleb128(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> offset: %lx\n", offset);
}

static void block1(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uint8_t size = bstream_u8(&contrib->sdies);
        bstream_advance(&contrib->sdies, size);
        contrib_debug_lprintf(contrib, "        -> size: 0x%02x\n", size);
}

static void block2(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uint16_t size = bstream_u16(&contrib->sdies);
        bstream_advance(&contrib->sdies, size);
        contrib_debug_lprintf(contrib, "        -> size: 0x%04x\n", size);
}

static void block4(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uint32_t size = bstream_u32(&contrib->sdies);
        bstream_advance(&contrib->sdies, size);
        contrib_debug_lprintf(contrib, "        -> size: 0x%08x\n", size);
}

static void block(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uleb128 size = bstream_uleb128(&contrib->sdies);
        bstream_advance(&contrib->sdies, size);
        contrib_debug_lprintf(contrib, "        -> size: 0x%016x\n", size);
}

static void udata(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const uleb128 size = bstream_uleb128(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> size: 0x%016x\n", size);
}

static void sdata(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const sleb128 size = bstream_sleb128(&contrib->sdies);
        contrib_debug_lprintf(contrib, "        -> size: 0x%016x (%ld)\n", size, size);
}

static void indirect(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        bstream_trap(&contrib->sdies, "Tried to execute the form parser for form DW_FORM_indirect");
}

static void strx(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const doff_t off_off = bstream_uleb128(&contrib->sdies);
        const doff_t str_off = get_string_offset(contrib, off_off);
        const char *string = get_string(contrib, str_off);
        contrib_debug_lprintf(contrib, "        -> string: \"%s\"\n", string);
}

static void strx1(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const doff_t off_off = bstream_u8(&contrib->sdies);
        const doff_t str_off = get_string_offset(contrib, off_off);
        const char *string = get_string(contrib, str_off);
        contrib_debug_lprintf(contrib, "        -> string: \"%s\"\n", string);
}

static void strx2(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const doff_t off_off = bstream_u16(&contrib->sdies);
        const doff_t str_off = get_string_offset(contrib, off_off);
        const char *string = get_string(contrib, str_off);
        contrib_debug_lprintf(contrib, "        -> string: \"%s\"\n", string);
}

static void strx3(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const doff_t off_off = bstream_u24(&contrib->sdies);
        const doff_t str_off = get_string_offset(contrib, off_off);
        const char *string = get_string(contrib, str_off);
        contrib_debug_lprintf(contrib, "        -> string: \"%s\"\n", string);
}

static void strx4(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        const doff_t off_off = bstream_u32(&contrib->sdies);
        const doff_t str_off = get_string_offset(contrib, off_off);
        const char *string = get_string(contrib, str_off);
        contrib_debug_lprintf(contrib, "        -> string: \"%s\"\n", string);
}

static void addrx(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        bstream_trap(&contrib->sdies, "%s form parser not implemented", __FUNCTION__);
}

static void addrx1(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        // TODO: implement
        bstream_trap(&contrib->sdies, "%s form parser not implemented", __FUNCTION__);
}

static void addrx2(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        // TODO: implement
        bstream_trap(&contrib->sdies, "%s form parser not implemented", __FUNCTION__);
}

static void addrx3(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        // TODO: implement
        bstream_trap(&contrib->sdies, "%s form parser not implemented", __FUNCTION__);
}

static void addrx4(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        // TODO: implement
        bstream_trap(&contrib->sdies, "%s form parser not implemented", __FUNCTION__);
}

static void ref_sup4(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        // TODO: implement
        bstream_trap(&contrib->sdies, "%s form parser not implemented", __FUNCTION__);
}

static void ref_sup8(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        // TODO: implement
        bstream_trap(&contrib->sdies, "%s form parser not implemented", __FUNCTION__);
}

static void strp_sup(struct parse_contrib_t *contrib, struct abbrev_attr_t)
{
        // TODO: implement
        bstream_trap(&contrib->sdies, "%s form parser not implemented", __FUNCTION__);
}

static const struct form_parser_table_entry_t FORM_PARSERS[] = {
        {DW_FORM_addr,   addr},
        {DW_FORM_block2, block2},
        {DW_FORM_block4, block4},
        {DW_FORM_data2,  data2},
        {DW_FORM_data4,  data4},
        {DW_FORM_data8,  data8},
        {DW_FORM_string, string},
        {DW_FORM_block, block},
        {DW_FORM_block1, block1},
        {DW_FORM_data1,  data1},
        {DW_FORM_flag, flag},
        {DW_FORM_sdata, sdata},
        {DW_FORM_strp,   strp},
        {DW_FORM_udata, udata},
        {DW_FORM_ref_addr, ref_addr},
        {DW_FORM_ref1, ref1},
        {DW_FORM_ref2, ref2},
        {DW_FORM_ref4, ref4},
        {DW_FORM_ref8, ref8},
        {DW_FORM_ref_udata, ref_udata},
        {DW_FORM_indirect, indirect},
        {DW_FORM_sec_offset, sec_offset},
        {DW_FORM_exprloc, exprloc},
        {DW_FORM_flag_present, flag_present},
        {DW_FORM_strx, strx},
        {DW_FORM_addrx, addrx},
        {DW_FORM_ref_sup4, ref_sup4},
        {DW_FORM_strp_sup, strp_sup},
        {DW_FORM_data16, data16},
        {DW_FORM_line_strp, line_strp},
        {DW_FORM_ref_sig8, ref_sig8},
        {DW_FORM_implicit_const, implicit_const},
        {DW_FORM_loclistx, loclistx},
        {DW_FORM_rnglistx, rnglistx},
        {DW_FORM_ref_sup8, ref_sup8},
        {DW_FORM_strx1, strx1},
        {DW_FORM_strx2, strx2},
        {DW_FORM_strx3, strx3},
        {DW_FORM_strx4, strx4},
        {DW_FORM_addrx1, addrx1},
        {DW_FORM_addrx2, addrx2},
        {DW_FORM_addrx3, addrx3},
        {DW_FORM_addrx4, addrx4},
};

static const struct form_parser_table_entry_t *parser_table_get()
{
        return FORM_PARSERS;
}

static size_t parser_table_size()
{
        return sizeof(FORM_PARSERS) / sizeof(*FORM_PARSERS);
}
