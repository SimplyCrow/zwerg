#ifndef LIBDWARF_STRING_TABLES_H
#define LIBDWARF_STRING_TABLES_H

#include <stdint.h>

#include "encoding.h"

const char *get_str_tag_encoding(int tag);
const char *get_str_attribute_encoding(int attribute);
const char *get_str_attribute_form_class(int class);
const char *get_str_attribute_form_encoding(int form);

#endif // LIBDWARF_STRING_TABLES_H
