#include "bstream.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <setjmp.h>

#include "leb128.h"

[[noreturn]] static void panic_exited_trap()
{
        fprintf(stderr, "PANIC: exited trap\n");
        exit(EXIT_FAILURE);
}

static void bstream_trap_on_out_of_bounds(struct bstream_t *stream, size_t bytes)
{
        if(bytes > stream->length || stream->pos > stream->length - bytes) {
                bstream_trap(stream, "Tried to access data beyond range");
        }
}

static size_t bstream_check_leb128_with_trap(struct bstream_t *stream)
{
        size_t  count = 0;
        uint8_t byte  = 0;
        do {
                bstream_trap_on_out_of_bounds(stream, count + 1);
                byte = stream->data[stream->pos + count];
                ++count;
        } while(__builtin_expect(byte & 0x80, 0));
        return count;
}

static uint8_t *bstream_current_pointer(struct bstream_t *stream)
{
        return stream->data + stream->pos;
}

bool bstream_init(struct bstream_t *stream, uint8_t *data, size_t length)
{
        assert(stream && data);
        stream->data = data;
        stream->length = length;
        stream->pos = 0;
        stream->err_message = NULL;
        return true;
}

size_t bstream_tell(struct bstream_t *stream)
{
        return stream->pos;
}

void bstream_set_pos(struct bstream_t *stream, size_t new_pos)
{
        if(stream->length <= new_pos) {
                bstream_trap(stream, "Tried to set pos to invalid value (out of bounds)");
        }
        stream->pos = new_pos;
}

[[noreturn]] void bstream_trap(struct bstream_t *stream, const char *error_message)
{
        stream->err_message = error_message;
        longjmp(stream->err_return, 1);
        panic_exited_trap();
}

void bstream_advance(struct bstream_t *stream, size_t bytes)
{
        bstream_trap_on_out_of_bounds(stream, bytes);
        stream->pos += bytes;
}

uint8_t bstream_peek_u8(struct bstream_t *stream)
{
        const size_t size = sizeof(uint8_t);
        bstream_trap_on_out_of_bounds(stream, size);
        uint8_t *p = bstream_current_pointer(stream);
        return (uint8_t)p[0];
}

uint16_t bstream_peek_u16(struct bstream_t *stream)
{
        const size_t size = sizeof(uint16_t);
        bstream_trap_on_out_of_bounds(stream, size);
        uint8_t *p = bstream_current_pointer(stream);
        return ((uint16_t)p[0])
                | ((uint16_t)p[1] << 8);
}

uint32_t bstream_peek_u32(struct bstream_t *stream)
{
        const size_t size = sizeof(uint32_t);
        bstream_trap_on_out_of_bounds(stream, size);
        uint8_t *p = bstream_current_pointer(stream);
        return ((uint32_t)p[0])
                | ((uint32_t)p[1] << 8)
                | ((uint32_t)p[2] << 16)
                | ((uint32_t)p[3] << 24);
}

uint64_t bstream_peek_u64(struct bstream_t *stream)
{
        const size_t size = sizeof(uint64_t);
        bstream_trap_on_out_of_bounds(stream, size);
        uint8_t *p = bstream_current_pointer(stream);
        return ((uint64_t)p[0])
                | ((uint64_t)p[1] << 8)
                | ((uint64_t)p[2] << 16)
                | ((uint64_t)p[3] << 24)
                | ((uint64_t)p[4] << 32)
                | ((uint64_t)p[5] << 40)
                | ((uint64_t)p[6] << 48)
                | ((uint64_t)p[7] << 56);
}

uint8_t bstream_u8(struct bstream_t *stream)
{
        const uint8_t value = bstream_peek_u8(stream);
        stream->pos += sizeof(value);
        return value;
}

uint16_t bstream_u16(struct bstream_t *stream)
{
        const uint16_t value = bstream_peek_u16(stream);
        stream->pos += sizeof(value);
        return value;
}

uint32_t bstream_u32(struct bstream_t *stream)
{
        const uint32_t value = bstream_peek_u32(stream);
        stream->pos += sizeof(value);
        return value;
}

uint64_t bstream_u64(struct bstream_t *stream)
{
        const uint64_t value = bstream_peek_u64(stream);
        stream->pos += sizeof(value);
        return value;
}

uint64_t bstream_uleb128(struct bstream_t *stream)
{
        const size_t size = bstream_check_leb128_with_trap(stream);
        if(size > sizeof(uint64_t)) {
                bstream_trap(stream, "unsigned leb128 over 8 bytes");
        }
        uint64_t value = 0;
        uleb128_decode(bstream_current_pointer(stream), &value);
        stream->pos += size;
        //printf("size = %zu, pos = %zu, value = %ld\n", size, stream->pos, value);
        return value;
}

int64_t bstream_sleb128(struct bstream_t *stream)
{
        const size_t size = bstream_check_leb128_with_trap(stream);
        if(size > sizeof(int64_t)) {
                bstream_trap(stream, "signed leb128 over 8 bytes");
        }
        int64_t value = 0;
        sleb128_decode(bstream_current_pointer(stream), &value);
        stream->pos += size;
        return value;
}

const char *bstream_string(struct bstream_t *stream)
{
        const char *p = (const char *)bstream_current_pointer(stream);
        char current_char = '\0';
        do {
                current_char = (char)bstream_u8(stream);
        } while(current_char != '\0');
        return p;
}
