// SPDX-License-Identifier: GPL-3.0-or-later
/*
 *  arena allocator
 *
 *  Copyright (C) 2026 Jonathan Kowalski <jonathan.kowalski2306@gmail.com>
 */

#ifndef LIBDWARF_ARENA_H
#define LIBDWARF_ARENA_H

#include <stdint.h>
#include <stddef.h>

struct arena_t {
        uint8_t *base;
        size_t   size;
        size_t   offset;
        struct arena_t *next;
};

bool  arena_create(struct arena_t *arena, size_t size);
void *arena_alloc(struct arena_t *arena, size_t size);
void  arena_free(struct arena_t *arena);

#endif // LIBDWARF_ARENA_H
