#ifndef LIBDWARF_BSTREAM_H
#define LIBDWARF_BSTREAM_H

#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>

struct bstream_t {
        uint8_t *data;
        size_t length;
        size_t pos;
        jmp_buf    err_return;
        const char *err_message;
};

bool bstream_init(struct bstream_t *stream, uint8_t *data, size_t length);

size_t   bstream_tell(struct bstream_t *stream);
void     bstream_set_pos(struct bstream_t *stream, size_t new_pos);
size_t   bstream_left(struct bstream_t *stream);

size_t bstream_trim_front(struct bstream_t *stream);

[[noreturn]] void bstream_trap(struct bstream_t *stream, const char *error_message);

uint8_t *bstream_advance(struct bstream_t *stream, size_t bytes);

uint8_t  bstream_peek_u8(struct bstream_t *stream);
uint16_t bstream_peek_u16(struct bstream_t *stream);
uint32_t bstream_peek_u32(struct bstream_t *stream);
uint64_t bstream_peek_u64(struct bstream_t *stream);

uint8_t  bstream_u8(struct bstream_t *stream);
uint16_t bstream_u16(struct bstream_t *stream);
uint32_t bstream_u32(struct bstream_t *stream);
uint64_t bstream_u64(struct bstream_t *stream);

uint64_t bstream_uleb128(struct bstream_t *stream);
int64_t  bstream_sleb128(struct bstream_t *stream);

const char *bstream_string(struct bstream_t *stream);

#endif // LIBDWARF_BSTREAM_H
