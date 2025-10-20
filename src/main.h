#ifndef CGOL_MAIN_H
#define CGOL_MAIN_H

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
#endif //CGOL_MAIN_H