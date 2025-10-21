#ifndef CGOL_TYPES_H
#define CGOL_TYPES_H
#include <stdbool.h>
#include <stddef.h>

typedef struct Vector2i {
    int x;
    int y;
} Vector2i;

typedef Vector2i ChunkId;
typedef Vector2i CellId;

typedef struct GlobalCellId {
    ChunkId chunkId;
    CellId cellId;
} GlobalCellId;

typedef struct Cell {
    bool alive;
    bool alive_next_gen;
} Cell;

typedef struct Chunk {
    ChunkId cid;
    Cell *cells;
} Chunk;

typedef struct ChunkDynamicArray {
    Chunk *items;
    size_t count;
    size_t capacity;
} ChunkDynamicArray;

#endif //CGOL_TYPES_H