#ifndef LIBDWARF_STRING_TABLES_H
#define LIBDWARF_STRING_TABLES_H

#include <stdint.h>

const char *get_str_tag_encoding(uint64_t tag);
const char *get_str_attribute_encoding(uint64_t attribute);
const char *get_str_attribute_form_encoding(uint64_t form);

#endif // LIBDWARF_STRING_TABLES_H
