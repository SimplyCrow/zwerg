// SPDX-License-Identifier: GPL-3.0-or-later
/*
 *  dwarf form parsing
 *
 *  Copyright (C) 2026 Jonathan Kowalski <jonathan.kowalski2306@gmail.com>
 */

#ifndef LIBDWARF_FORM_PARSER_H
#define LIBDWARF_FORM_PARSER_H

#include <libdwarf/dwarf.h>

#include "die_tree.h"
#include "encoding.h"
#include "abbrev.h"

typedef void (*form_parser_t)(struct parse_contrib_t*, struct abbrev_attr_t);
form_parser_t get_form_parser(did_t form);

#endif // LIBDWARF_FORM_PARSER_H
