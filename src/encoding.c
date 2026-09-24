#include "encoding.h"

#include <stddef.h>

#define ARRAY_SIZE(ar) (sizeof(ar) / sizeof(*ar))

// TODO: handle vendor specific ranges


/*
 * Note on DWARF 32 bit and 64 bit:
 * Section offsets (DW_FORM_strp, DW_FORM_ref_addr, DW_FORM_sec_offset,
 * DW_FORM_strp_sup, DW_FORM_line_strp) are set to 4 bytes for 32-bit DWARF.
 * In 64-bit DWARF, these 5 section offset forms would occupy 8 bytes instead.
 */
static const struct dwarf_attribute_form_info_mapping FORM_CLASS_MAPPING[] = {
        {0, DW_FORM_CLASS_UNKNOWN, DW_ATT_FORM_ARG_SIZE_INVALID},
        {DW_FORM_addr, DW_FORM_CLASS_ADDRESS, DW_ATT_FORM_ARG_SIZE_ARCH_POINTER},
        {DW_FORM_block2, DW_FORM_CLASS_BLOCK, DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE2},
        {DW_FORM_block4, DW_FORM_CLASS_BLOCK, DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE4},
        {DW_FORM_data2, DW_FORM_CLASS_CONSTANT, 2},
        {DW_FORM_data4, DW_FORM_CLASS_CONSTANT, 4},
        {DW_FORM_data8, DW_FORM_CLASS_CONSTANT, 8},
        {DW_FORM_string, DW_FORM_CLASS_STRING, DW_ATT_FORM_ARG_SIZE_STRING},
        {DW_FORM_block, DW_FORM_CLASS_BLOCK, DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZEX},
        {DW_FORM_block1, DW_FORM_CLASS_BLOCK, DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE1},
        {DW_FORM_data1, DW_FORM_CLASS_CONSTANT, 1},
        {DW_FORM_flag, DW_FORM_CLASS_FLAG, 1},
        {DW_FORM_sdata, DW_FORM_CLASS_CONSTANT, DW_ATT_FORM_ARG_SIZE_SLEB128},
        {DW_FORM_strp, DW_FORM_CLASS_STRING, 4},
        {DW_FORM_udata, DW_FORM_CLASS_CONSTANT, DW_ATT_FORM_ARG_SIZE_ULEB128},
        {DW_FORM_ref_addr, DW_FORM_CLASS_REFERENCE, 4},
        {DW_FORM_ref1, DW_FORM_CLASS_REFERENCE, 1},
        {DW_FORM_ref2, DW_FORM_CLASS_REFERENCE, 2},
        {DW_FORM_ref4, DW_FORM_CLASS_REFERENCE, 4},
        {DW_FORM_ref8, DW_FORM_CLASS_REFERENCE, 8},
        {DW_FORM_ref_udata, DW_FORM_CLASS_REFERENCE, DW_ATT_FORM_ARG_SIZE_ULEB128},
        {DW_FORM_indirect, DW_FORM_CLASS_INDIRECT, DW_ATT_FORM_ARG_SIZE_INDIRECT_FORM},
        {DW_FORM_sec_offset, DW_FORM_CLASS_SEC_OFFSET, 4},
        {DW_FORM_exprloc, DW_FORM_CLASS_EXPRLOC, DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZEX},
        {DW_FORM_flag_present, DW_FORM_CLASS_FLAG, 0},
        {DW_FORM_strx, DW_FORM_CLASS_STRING, DW_ATT_FORM_ARG_SIZE_ULEB128},
        {DW_FORM_addrx, DW_FORM_CLASS_ADDRESS, DW_ATT_FORM_ARG_SIZE_ULEB128},
        {DW_FORM_ref_sup4, DW_FORM_CLASS_REFERENCE, 4},
        {DW_FORM_strp_sup, DW_FORM_CLASS_STRING, 4},
        {DW_FORM_data16, DW_FORM_CLASS_CONSTANT, 16},
        {DW_FORM_line_strp, DW_FORM_CLASS_STRING, 4},
        {DW_FORM_ref_sig8, DW_FORM_CLASS_REFERENCE, 8},
        {DW_FORM_implicit_const, DW_FORM_CLASS_CONSTANT, 0},
        {DW_FORM_loclistx, DW_FORM_CLASS_LOCLIST, DW_ATT_FORM_ARG_SIZE_ULEB128},
        {DW_FORM_rnglistx, DW_FORM_CLASS_RNGLIST, DW_ATT_FORM_ARG_SIZE_ULEB128},
        {DW_FORM_ref_sup8, DW_FORM_CLASS_REFERENCE, 8},
        {DW_FORM_strx1, DW_FORM_CLASS_STRING, 1},
        {DW_FORM_strx2, DW_FORM_CLASS_STRING, 2},
        {DW_FORM_strx3, DW_FORM_CLASS_STRING, 3},
        {DW_FORM_strx4, DW_FORM_CLASS_STRING, 4},
        {DW_FORM_addrx1, DW_FORM_CLASS_ADDRESS, 1},
        {DW_FORM_addrx2, DW_FORM_CLASS_ADDRESS, 2},
        {DW_FORM_addrx3, DW_FORM_CLASS_ADDRESS, 3},
        {DW_FORM_addrx4, DW_FORM_CLASS_ADDRESS, 4},
};

struct dwarf_attribute_form_info_mapping
get_mapping_form_info(enum dwarf_attribute_form form)
{
        if(ARRAY_SIZE(FORM_CLASS_MAPPING) == 0) {
                return FORM_CLASS_MAPPING[0];
        }

        size_t low = 0;
        size_t high = ARRAY_SIZE(FORM_CLASS_MAPPING);

        while(low < high) {
                size_t mid = low + (high - low) / 2;

                if(FORM_CLASS_MAPPING[mid].form == form) {
                        return FORM_CLASS_MAPPING[mid];
                }

                if(FORM_CLASS_MAPPING[mid].form < form) {
                        low  = mid + 1;
                } else {
                        high = mid;
                }
        }

        return FORM_CLASS_MAPPING[0];
}

enum dwarf_attribute_form_class
get_mapping_form_class(enum dwarf_attribute_form form)
{
        return get_mapping_form_info(form).class;
}

uint64_t get_mapping_form_argument_size(enum dwarf_attribute_form form)
{
        return get_mapping_form_info(form).argument_size;
}
