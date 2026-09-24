#ifndef LIBDWARF_ENCODING_H
#define LIBDWARF_ENCODING_H

#include <stdint.h>

enum dwarf_unit_hdr_type {
        DW_UT_compile = 0x01,
        DW_UT_type = 0x02,
        DW_UT_partial = 0x03,
        DW_UT_skeleton = 0x04,
        DW_UT_split_compile = 0x05,
        DW_UT_split_type = 0x06,
        DW_UT_lo_user = 0x80,
        DW_UT_high_user = 0xff,
};

enum dwarf_tag {
        DW_TAG_array_type = 0x0001,
        DW_TAG_class_type = 0x0002,
        DW_TAG_entry_point = 0x0003,
        DW_TAG_enumeration_type = 0x0004,
        DW_TAG_formal_parameter = 0x0005,
        DW_TAG_imported_declaration = 0x0008,
        DW_TAG_label = 0x000a,
        DW_TAG_lexical_block = 0x000b,
        DW_TAG_member = 0x000d,
        DW_TAG_pointer_type = 0x000f,
        DW_TAG_reference_type = 0x0010,
        DW_TAG_compile_unit = 0x0011,
        DW_TAG_string_type = 0x0012,
        DW_TAG_structure_type = 0x0013,
        DW_TAG_subroutine_type = 0x0015,
        DW_TAG_typedef = 0x0016,
        DW_TAG_union_type = 0x0017,
        DW_TAG_unspecified_parameters = 0x0018,
        DW_TAG_variant = 0x0019,
        DW_TAG_common_block = 0x001a,
        DW_TAG_common_inclusion = 0x001b,
        DW_TAG_inheritance = 0x001c,
        DW_TAG_inlined_subroutine = 0x001d,
        DW_TAG_module = 0x001e,
        DW_TAG_ptr_to_member_type = 0x001f,
        DW_TAG_set_type = 0x0020,
        DW_TAG_subrange_type = 0x0021,
        DW_TAG_with_stmt = 0x0022,
        DW_TAG_access_declaration = 0x0023,
        DW_TAG_base_type = 0x0024,
        DW_TAG_catch_block = 0x0025,
        DW_TAG_const_type = 0x0026,
        DW_TAG_constant = 0x0027,
        DW_TAG_enumerator = 0x0028,
        DW_TAG_file_type = 0x0029,
        DW_TAG_friend = 0x002a,
        DW_TAG_namelist = 0x002b,
        DW_TAG_namelist_item = 0x002c,
        DW_TAG_packed_type = 0x002d,
        DW_TAG_subprogram = 0x002e,
        DW_TAG_template_type_parameter = 0x002f, DW_TAG_template_value_parameter = 0x0030, DW_TAG_thrown_type = 0x0031, DW_TAG_try_block = 0x0032,
        DW_TAG_variant_part = 0x0033,
        DW_TAG_variable = 0x0034,
        DW_TAG_volatile_type = 0x0035,
        DW_TAG_dwarf_procedure = 0x0036,
        DW_TAG_restrict_type = 0x0037,
        DW_TAG_interface_type = 0x0038,
        DW_TAG_namespace = 0x0039,
        DW_TAG_imported_module = 0x003a,
        DW_TAG_unspecified_type = 0x003b,
        DW_TAG_partial_unit = 0x003c,
        DW_TAG_imported_unit = 0x003d,
        DW_TAG_condition = 0x003f,
        DW_TAG_shared_type = 0x0040,
        DW_TAG_type_unit = 0x0041,
        DW_TAG_rvalue_reference_type = 0x0042,
        DW_TAG_template_alias = 0x0043,
        DW_TAG_coarray_type = 0x0044,
        DW_TAG_generic_subrange = 0x0045,
        DW_TAG_dynamic_type = 0x0046,
        DW_TAG_atomic_type = 0x0047,
        DW_TAG_call_site = 0x0048,
        DW_TAG_call_site_parameter = 0x0049,
        DW_TAG_skeleton_unit = 0x004a,
        DW_TAG_immutable_type = 0x004b,
        DW_TAG_lo_user = 0x4080,
        DW_TAG_hi_user = 0xffff,
};

enum dwarf_attribute_type {
        DW_AT_sibling = 0x0001,
        DW_AT_location = 0x0002,
        DW_AT_name = 0x0003,
        DW_AT_ordering = 0x0009,
        DW_AT_byte_size = 0x000b,
        DW_AT_bit_size = 0x000d,
        DW_AT_stmt_list = 0x0010,
        DW_AT_low_pc = 0x0011,
        DW_AT_high_pc = 0x0012,
        DW_AT_language = 0x0013,
        DW_AT_discr = 0x0015,
        DW_AT_discr_value = 0x0016,
        DW_AT_visibility = 0x0017,
        DW_AT_import = 0x0018,
        DW_AT_string_length = 0x0019,
        DW_AT_common_reference = 0x001a,
        DW_AT_comp_dir = 0x001b,
        DW_AT_const_value = 0x001c,
        DW_AT_containing_type = 0x001d,
        DW_AT_default_value = 0x001e,
        DW_AT_inline = 0x0020,
        DW_AT_is_optional = 0x0021,
        DW_AT_lower_bound = 0x0022,
        DW_AT_producer = 0x0025,
        DW_AT_prototyped = 0x0027,
        DW_AT_return_addr = 0x002a,
        DW_AT_start_scope = 0x002c,
        DW_AT_bit_stride = 0x002e,
        DW_AT_upper_bound = 0x002f,
        DW_AT_abtract_origin = 0x0031,
        DW_AT_accessibility = 0x0032,
        DW_AT_address_class = 0x0033,
        DW_AT_artificial = 0x0034,
        DW_AT_base_types = 0x0035,
        DW_AT_calling_convention = 0x0036,
        DW_AT_count = 0x0037,
        DW_AT_data_member_location = 0x0038,
        DW_AT_decl_column = 0x0039,
        DW_AT_decl_file = 0x003a,
        DW_AT_decl_line = 0x003b,
        DW_AT_declaration = 0x003c,
        DW_AT_discr_list = 0x003d,
        DW_AT_encoding = 0x003e,
        DW_AT_external = 0x003f,
        DW_AT_frame_base = 0x0040,
        DW_AT_friend = 0x0041,
        DW_AT_identifier_case = 0x0042,
        DW_AT_namelist_item = 0x0044,
        DW_AT_priority = 0x0045,
        DW_AT_segment = 0x0046,
        DW_AT_specification = 0x0047,
        DW_AT_static_link = 0x0048,
        DW_AT_type = 0x0049,
        DW_AT_use_location = 0x004a,
        DW_AT_variable_parameter = 0x004b,
        DW_AT_virtuality = 0x004c,
        DW_AT_vtable_elem_location = 0x004d,
        DW_AT_allocated = 0x004e,
        DW_AT_associated = 0x004f,
        DW_AT_data_location = 0x0050,
        DW_AT_byte_stride = 0x0051,
        DW_AT_entry_pc = 0x0052,
        DW_AT_use_UTF8 = 0x0053,
        DW_AT_extension = 0x0054,
        DW_AT_ranges = 0x0055,
        DW_AT_trampoline = 0x0056,
        DW_AT_call_column = 0x0057,
        DW_AT_call_file = 0x0058,
        DW_AT_call_line = 0x0059,
        DW_AT_description = 0x005a,
        DW_AT_binary_scale = 0x005b,
        DW_AT_decimal_scale = 0x005c,
        DW_AT_small = 0x005d,
        DW_AT_decimal_sign = 0x005e,
        DW_AT_digit_count = 0x005f,
        DW_AT_picture_string = 0x0060,
        DW_AT_mutable = 0x0061,
        DW_AT_threads_scaled = 0x0062,
        DW_AT_explicit = 0x0063,
        DW_AT_object_pointer = 0x0064,
        DW_AT_endianity = 0x0065,
        DW_AT_elemental = 0x0066,
        DW_AT_pure = 0x0067,
        DW_AT_recursive = 0x0068,
        DW_AT_signature = 0x0069,
        DW_AT_main_subprogram = 0x006a,
        DW_AT_data_bit_offset = 0x006b,
        DW_AT_const_expr = 0x006c,
        DW_AT_enum_class = 0x006d,
        DW_AT_linkage_name = 0x006e,
        DW_AT_string_length_bit_size = 0x006f,
        DW_AT_string_length_byte_size = 0x0070,
        DW_AT_rank = 0x0071,
        DW_AT_str_offsets_base = 0x0072,
        DW_AT_addr_base = 0x0073,
        DW_AT_rnglists_base = 0x0074,
        DW_AT_dwo_name = 0x0076,
        DW_AT_reference = 0x0077,
        DW_AT_rvalue_reference = 0x0078,
        DW_AT_macros = 0x0079,
        DW_AT_call_all_calls = 0x007a,
        DW_AT_call_all_source_calls = 0x007b,
        DW_AT_call_all_tail_calls = 0x007c,
        DW_AT_call_return_pc = 0x007d,
        DW_AT_call_value = 0x007e,
        DW_AT_call_origin = 0x007f,
        DW_AT_call_parameter = 0x0080,
        DW_AT_call_pc = 0x0081,
        DW_AT_call_tail_call = 0x0082,
        DW_AT_call_target = 0x0083,
        DW_AT_call_target_clobbered = 0x0084,
        DW_AT_call_data_location = 0x0085,
        DW_AT_call_data_value = 0x0086,
        DW_AT_noreturn = 0x0087,
        DW_AT_alignment = 0x0088,
        DW_AT_export_symbols = 0x0089,
        DW_AT_deleted = 0x008a,
        DW_AT_defaulted = 0x008b,
        DW_AT_loclists_base = 0x008c,
        DW_AT_lo_user = 0x2000,
        DW_AT_hi_user = 0x3fff,
};

enum dwarf_attribute_form_class {
        DW_FORM_CLASS_UNKNOWN,
        DW_FORM_CLASS_INDIRECT,
        DW_FORM_CLASS_SEC_OFFSET, // addrptr, lineptr, loclist, loclistsptr, macptr, rnglist, rnglistsptr, stroffsetsptr
        DW_FORM_CLASS_ADDRESS,
        DW_FORM_CLASS_ADDRPTR,
        DW_FORM_CLASS_BLOCK,
        DW_FORM_CLASS_CONSTANT,
        DW_FORM_CLASS_EXPRLOC,
        DW_FORM_CLASS_FLAG,
        DW_FORM_CLASS_LINEPTR,
        DW_FORM_CLASS_LOCLIST,
        DW_FORM_CLASS_LOCLISTSPTR,
        DW_FORM_CLASS_MACPTR,
        DW_FORM_CLASS_RNGLIST,
        DW_FORM_CLASS_RNGLISTSPTR,
        DW_FORM_CLASS_REFERENCE,
        DW_FORM_CLASS_STRING,
        DW_FORM_CLASS_STROFFSETSPTR,
};

enum dwarf_attribute_form {
        DW_FORM_addr = 0x0001,
        DW_FORM_block2 = 0x0003,
        DW_FORM_block4 = 0x0004,
        DW_FORM_data2 = 0x0005,
        DW_FORM_data4 = 0x0006,
        DW_FORM_data8 = 0x0007,
        DW_FORM_string = 0x0008,
        DW_FORM_block = 0x0009,
        DW_FORM_block1 = 0x000a,
        DW_FORM_data1 = 0x000b,
        DW_FORM_flag = 0x000c,
        DW_FORM_sdata = 0x000d,
        DW_FORM_strp = 0x000e,
        DW_FORM_udata = 0x000f,
        DW_FORM_ref_addr = 0x0010,
        DW_FORM_ref1 = 0x0011,
        DW_FORM_ref2 = 0x0012,
        DW_FORM_ref4 = 0x0013,
        DW_FORM_ref8 = 0x0014,
        DW_FORM_ref_udata = 0x0015,
        DW_FORM_indirect = 0x0016,
        DW_FORM_sec_offset = 0x0017,
        DW_FORM_exprloc = 0x0018,
        DW_FORM_flag_present = 0x0019,
        DW_FORM_strx = 0x001a,
        DW_FORM_addrx = 0x001b,
        DW_FORM_ref_sup4 = 0x001c,
        DW_FORM_strp_sup = 0x001d,
        DW_FORM_data16 = 0x001e,
        DW_FORM_line_strp = 0x001f,
        DW_FORM_ref_sig8 = 0x0020,
        DW_FORM_implicit_const = 0x0021,
        DW_FORM_loclistx = 0x0022,
        DW_FORM_rnglistx = 0x0023,
        DW_FORM_ref_sup8 = 0x0024,
        DW_FORM_strx1 = 0x0025,
        DW_FORM_strx2 = 0x0026,
        DW_FORM_strx3 = 0x0027,
        DW_FORM_strx4 = 0x0028,
        DW_FORM_addrx1 = 0x0029,
        DW_FORM_addrx2 = 0x002a,
        DW_FORM_addrx3 = 0x002b,
        DW_FORM_addrx4 = 0x002c,
};

/* SPECIAL ATTRIBUTE FORM ARGUMENT SIZES
 *
 * DW_ATT_FORM_ARG_SIZE_INVALID:        invalid (undefined behavior)
 * DW_ATT_FORM_ARG_SIZE_ULEB128:        single unsigned LEB128
 * DW_ATT_FORM_ARG_SIZE_SLEB128:        single signed LEB128
 * DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZEX: single unsigned LEB128 defining buffer size after it
 * DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE1: DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE with 1 byte size, not LEB128
 * DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE2: DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE with 2 byte size, not LEB128
 * DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE4: DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE with 4 byte size, not LEB128
 * DW_ATT_FORM_ARG_SIZE_INDIRECT_FORM:  unsigned LEB128 defining form, parse respectivly
 * DW_ATT_FORM_ARG_SIZE_ARCH_POINTER:   machine dependent pointer size as defined in the header
 * DW_ATT_FORM_ARG_SIZE_STRING:         null terminated c string
 *
 */

#define DW_ATT_FORM_ARG_SIZE_INVALID        (UINT64_MAX - 0)
#define DW_ATT_FORM_ARG_SIZE_ULEB128        (UINT64_MAX - 1)
#define DW_ATT_FORM_ARG_SIZE_SLEB128        (UINT64_MAX - 2)
#define DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZEX (UINT64_MAX - 3)
#define DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE1 (UINT64_MAX - 4)
#define DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE2 (UINT64_MAX - 5)
#define DW_ATT_FORM_ARG_SIZE_INDIRECT_SIZE4 (UINT64_MAX - 6)
#define DW_ATT_FORM_ARG_SIZE_INDIRECT_FORM  (UINT64_MAX - 7)
#define DW_ATT_FORM_ARG_SIZE_ARCH_POINTER   (UINT64_MAX - 8)
#define DW_ATT_FORM_ARG_SIZE_STRING         (UINT64_MAX - 9)

struct dwarf_attribute_form_info_mapping {
        enum dwarf_attribute_form       form;
        enum dwarf_attribute_form_class class;
        uint64_t argument_size;
};

struct dwarf_attribute_form_info_mapping
get_mapping_form_info(enum dwarf_attribute_form form);

enum dwarf_attribute_form_class
get_mapping_form_class(enum dwarf_attribute_form form);

uint64_t get_mapping_form_argument_size(enum dwarf_attribute_form form);

/* === POSSIBLE PARAMETERS TYPES FOR FORMS
 *
 *      - pointer (machine dependent on compilation unit header)
 *      - LSB128 (unsigned and signed)
 *      - 1- 2- 4- 8- 16-byte data
 *      - fixed ptr/offset (32 bit in DWARF 32, 64 bit for DWARF 64)
 *      - buffers (1 byte, 2, byte, 4 byte and LEB128 byte addresses)
 *      - void
 *      - etc.
 *
 *      GENERAL FORM
 *      1. void
 *      2. fixed n byte
 *      3. first LEB128 that specifices following size
 *      4. first LEB128 that specifices attribute form than following 1. 2. 3.
 * */

#endif // LIBDWARF_ENCODING_H
