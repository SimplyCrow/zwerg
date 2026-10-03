// SPDX-License-Identifier: GPL-3.0-or-later
/*
 *  arena allocator
 *
 *  Copyright (C) 2026 Jonathan Kowalski <jonathan.kowalski2306@gmail.com>
 */

#include "arena.h"

#include <stdlib.h>

static inline bool arena_add_block(struct arena_t *arena, size_t minimum_size)
{
        struct arena_t *old_block = malloc(sizeof(*arena));
        if(!old_block) {
                return false;
        }
        *old_block = *arena;

        if(arena->size < minimum_size) {
                arena->size = minimum_size;
        }

        arena->base = malloc(arena->size);
        arena->offset = 0;
        arena->next = old_block;
        return arena->base;
}

bool arena_create(struct arena_t *arena, size_t size)
{
        arena->offset = 0;
        arena->next = NULL;
        arena->size = size;
        arena->base = malloc(arena->size);
        return arena->base;
}

void *arena_alloc(struct arena_t *arena, size_t size)
{
        if(arena->offset + size > arena->size) {
                arena_add_block(arena, size);
        }

        void *p = arena->base + arena->offset;
        arena->offset += size;
        return p;
}

void arena_free(struct arena_t *arena)
{
        while(arena != NULL) {
                free(arena->base);
                struct arena_t *next = arena->next;
                if(next != NULL) {
                        free(arena);
                }
                arena = next;
        }
}
