#include "cda.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "types.h"
#include "utils.h"

ChunkDynamicArray cdaInit() {
    Chunk *items = malloc(CDA_INITAL_CAPACITY * sizeof(Chunk));
    if (items == NULL) {
        fprintf(stderr, "Couldn't allocate, buy more RAM\n");
        exit(1);
    }
    memset(items, 0, CDA_INITAL_CAPACITY * sizeof(Chunk));
    return (ChunkDynamicArray){items, 0, CDA_INITAL_CAPACITY};
}

void cdaEnsureCapacity(ChunkDynamicArray cda[static 1], const size_t desiredCapacity) {
    if (desiredCapacity < cda->capacity) {
        return;
    }
    size_t newCapacity = cda->capacity;
    while (newCapacity < desiredCapacity) {
        newCapacity *= CDA_GROWTH_FACTOR;
    }
    Chunk *items = realloc(cda->items, newCapacity * sizeof(Chunk *));
    if (items == NULL) {
        fprintf(stderr, "Couldn't allocate, buy more RAM\n");
        exit(1);
    }
    cda->items = items;
    cda->capacity = newCapacity;
}

Chunk *cdaAppend(ChunkDynamicArray cda[static 1], const Chunk chunk) {
    cdaEnsureCapacity(cda, cda->count + 1);
    cda->items[cda->count++] = chunk;
    return &cda->items[cda->count - 1];
}

Chunk *cdaFind(const ChunkDynamicArray cda, const ChunkId cid) {
    // Using a hashmap would be the better solution here. For simplicity, I just
    // used linear search.
    for (int i = 0; i < cda.count; ++i) {
        if (cidEquals(cda.items[i].cid, cid)) {
            return &cda.items[i];
        }
    }
    return NULL;
}

void cdaFree(const ChunkDynamicArray cda) {
    for (size_t i = 0; i < cda.count; ++i) {
        free(cda.items[i].cells);
    }
    free(cda.items);
}

Chunk *cdaAddChunk(ChunkDynamicArray cda[static 1], const ChunkId cid) {
    Cell *cells = malloc(CHUNK_SIZE * CHUNK_SIZE * sizeof(Cell));
    if (cells == NULL) {
        fprintf(stderr, "Couldn't allocate, buy more RAM\n");
        exit(1);
    }
    memset(cells, 0, CHUNK_SIZE * CHUNK_SIZE * sizeof(Cell));
    return cdaAppend(cda, (Chunk){.cid = cid, .cells = cells});
}
