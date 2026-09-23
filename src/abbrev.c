#include "abbrev.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "string_tables.h"

static size_t count_entries(
                struct dwarf_buffer_t  abbrev_section,
                size_t offset
)
{
        size_t   count = 0;
        uint8_t *abbrev_table = abbrev_section.data + offset;
        size_t   abbrev_off = 0;
        while(abbrev_table[abbrev_off] != 0) {
                abbrev_off += uleb128_size(abbrev_table + abbrev_off);
                abbrev_off += uleb128_size(abbrev_table + abbrev_off);
                abbrev_off += 1; // has children byte

                uleb128 name, form;
                do {
                        abbrev_off += uleb128_decode(abbrev_table + abbrev_off, &name);
                        abbrev_off += uleb128_decode(abbrev_table + abbrev_off, &form);
                } while(name != 0x00 || form != 0x00);
                ++count;
        }
        return count;
}

static size_t count_attributes(uint8_t *abbrev_table, size_t abbrev_off)
{
        size_t count = 0;
        uleb128 name, form;
        do {
                abbrev_off += uleb128_decode(abbrev_table + abbrev_off, &name);
                abbrev_off += uleb128_decode(abbrev_table + abbrev_off, &form);
                if(name != 0x00 && form != 0x00) {
                        ++count;
                }
        } while(name != 0x00 || form != 0x00);
        return count;
}

bool abbrev_table_create(
                struct abbrev_table_t *table,
                struct dwarf_buffer_t  abbrev_section,
                size_t offset
)
{
        assert(table);
        assert(offset <= abbrev_section.size);

        struct abbrev_table_t t = {0};
        t.count = count_entries(abbrev_section, offset);
        t.entries = malloc(t.count * sizeof(*t.entries));
        printf("Found %zu abbreviation entries\n", t.count);

        uint8_t *abbrev_table = abbrev_section.data + offset;
        size_t   abbrev_off = 0;
        size_t   i = 0;
        while(abbrev_table[abbrev_off] != 0) {
                struct abbrev_entry_t *entry = &t.entries[i];
                abbrev_off += uleb128_decode(abbrev_table + abbrev_off, &entry->index);
                abbrev_off += uleb128_decode(abbrev_table + abbrev_off, &entry->tag);
                entry->children = (abbrev_table[abbrev_off++] == 0x01);

                entry->attr_count = count_attributes(abbrev_table, abbrev_off);
                entry->attrs = malloc(entry->attr_count * sizeof(*entry->attrs));

                size_t j = 0;
                uleb128 name = 0, form = 0;
                do {
                        abbrev_off += uleb128_decode(abbrev_table + abbrev_off, &name);
                        abbrev_off += uleb128_decode(abbrev_table + abbrev_off, &form);
                        if(name != 0 && form != 0) {
                                entry->attrs[j].name = name;
                                entry->attrs[j].form = form;
                                ++j;
                        }
                } while(name != 0x00 || form != 0x00);
                ++i;
        }

        for(size_t i = 0; i < t.count; ++i) {
                struct abbrev_entry_t *entry = &t.entries[i];
                printf(" * [0x%02lx]: tag: %s (0x%02lx) %s:\n", entry->index
                                , get_str_tag_encoding(entry->tag)
                                , entry->tag
                                , (entry->children == 0x00) ? "no children" : "children");
                for(size_t j = 0; j < entry->attr_count; ++j) {
                        uint64_t name = entry->attrs[j].name;
                        uint64_t form = entry->attrs[j].form;
                        printf("     - %s (0x%02lx) : %s (0x%02lx)\n"
                                        , get_str_attribute_encoding(name)
                                        , name
                                        , get_str_attribute_form_encoding(form)
                                        , form);
                }
        }

        *table = t;
        return false;
}

struct abbrev_entry_t *abbrev_table_get(
                struct abbrev_table_t *table,
                uint64_t index
)
{
        assert(table);
        for(size_t i = 0; i < table->count; ++i) {
                if(table->entries[i].index == index) {
                        return &table->entries[i];
                }
        }
        return NULL;
}
