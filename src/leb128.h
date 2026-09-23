#ifndef LIBDWARF_LEB128_H
#define LIBDWARF_LEB128_H

typedef uint64_t uleb128;
typedef int64_t  sleb128;

static inline size_t leb128_size(const uint8_t *src)
{
        size_t  count = 0;
        uint8_t byte  = 0;
        do {
                byte = src[count++];
        } while(__builtin_expect(byte & 0x80, 0));
        return count;
}

static inline size_t uleb128_decode(const uint8_t *src, uleb128 *out)
{
        size_t   count = 0;
        unsigned shift = 0;
        uint8_t  byte  = 0;

        uleb128 result = 0;
        do {
                byte = src[count++];
                result |= ((uleb128)(byte & 0x7f) << shift);
                shift += 7;
        } while(__builtin_expect(byte & 0x80, 0));

        *out = result;
        return count;
}

static inline size_t sleb128_decode(const uint8_t *src, sleb128 *out)
{
        size_t   count = 0;
        unsigned shift = 0;
        uint8_t  byte  = 0;

        uleb128 result = 0;
        do {
                byte = src[count++];
                result |= ((uleb128)(byte & 0x7f) << shift);
                shift += 7;
        } while(__builtin_expect(byte & 0x80, 0));

        if((shift < (sizeof(sleb128) * 8)) && (byte & 0x40))
                result |= (~(uleb128)0 << shift);

        *out = (sleb128)result;
        return count;
}

#endif // LIBDWARF_LEB128_H
