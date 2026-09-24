#include "abbrev.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "string_tables.h"
#include "bstream.h"

static void dump_abbrev_table(struct abbrev_table_t *table)
{
        printf("Abbreviation table (%zu):\n", table->count);
        for(size_t i = 0; i < table->count; ++i) {
                struct abbrev_entry_t *entry = &table->entries[i];
                printf(" * DAT [0x%02lx]: tag: %s (0x%02lx) %s:\n", entry->index
                                , get_str_tag_encoding(entry->tag)
                                , entry->tag
                                , (entry->children == 0x00) ? "no children" : "children");
                for(size_t j = 0; j < entry->attr_count; ++j) {
                        uint64_t name = entry->attrs[j].name;
                        uint64_t form = entry->attrs[j].form;
                        int64_t  implicit_const = entry->attrs[j].implicit_const;
                        printf("     - %s (0x%02lx) : %s (0x%02lx) [%s] "
                                        , get_str_attribute_encoding(name)
                                        , name
                                        , get_str_attribute_form_encoding(form)
                                        , form
                                        , get_str_attribute_form_class(
                                                get_mapping_form_class(form))
                                        );

                        if(form == DW_FORM_implicit_const) {
                                printf("[%ld] ", implicit_const);
                        }
                        printf("\n");
                }
        }
}

static size_t count_attributes(struct bstream_t *stream)
{
        size_t   count = 0;
        uint64_t name, form;
        do {
                name = bstream_uleb128(stream);
                form = bstream_uleb128(stream);
                if(form == DW_FORM_implicit_const) {
                        bstream_sleb128(stream); // implicit const
                }
                ++count;
        } while(name != 0x00 || form != 0x00);
        return count - 1; // off by one because of the end mark
}

static size_t count_entries(struct bstream_t *stream)
{
        size_t count = 0;
        while(bstream_peek_u8(stream)) {
                bstream_uleb128(stream); // index
                bstream_uleb128(stream); // tag
                bstream_u8(stream);      // children

                count_attributes(stream);

                ++count;
        }
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

        struct bstream_t stream;
        bstream_init(&stream, abbrev_section.data, abbrev_section.size);

        int error = setjmp(stream.err_return);
        if(error != 0) {
                fprintf(stderr, "[ERROR] [AT 0x%lx (%zu)] [SIZE 0x%lx (%zu)] Could not parse abbreviation table %d: %s\n"
                                , stream.pos, stream.pos
                                , stream.length, stream.length
                                , error,
                                stream.err_message ? stream.err_message : "NO ERROR MESSAGE");
                return false;
        }


        struct abbrev_table_t t = {0};

        const size_t pos = bstream_tell(&stream);
        t.count = count_entries(&stream);
        bstream_set_pos(&stream, pos);

        t.entries = malloc(t.count * sizeof(*t.entries));
        printf("Found %zu abbreviation entries\n", t.count);

        size_t   i = 0;
        while(bstream_peek_u8(&stream)) {
                struct abbrev_entry_t *entry = &t.entries[i];
                entry->index    = bstream_uleb128(&stream);
                entry->tag      = bstream_uleb128(&stream);
                entry->children = (bstream_u8(&stream) == 0x01);

                const size_t pos = bstream_tell(&stream);
                entry->attr_count = count_attributes(&stream);
                bstream_set_pos(&stream, pos);

                entry->attrs      = malloc(entry->attr_count * sizeof(*entry->attrs));

                size_t j = 0;
                uint64_t name = 0, form = 0;
                int64_t  implicit_const = 0;
                do {
                        name = bstream_uleb128(&stream);
                        form = bstream_uleb128(&stream);
                        if(form == DW_FORM_implicit_const) {
                                implicit_const = bstream_sleb128(&stream);
                        }
                        if(name != 0 && form != 0) {
                                entry->attrs[j].name = name;
                                entry->attrs[j].form = form;
                                entry->attrs[j].implicit_const = implicit_const;
                                ++j;
                        }
                } while(name != 0x00 || form != 0x00);
                ++i;
        }

        //dump_abbrev_table(&t);

        *table = t;
        return true;
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
