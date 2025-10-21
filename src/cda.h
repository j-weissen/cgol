#ifndef CGOL_CDA_H
#define CGOL_CDA_H

#include "types.h"


ChunkDynamicArray cdaInit();
void cdaEnsureCapacity(ChunkDynamicArray cda[static 1], size_t desiredCapacity);
Chunk *cdaAppend(ChunkDynamicArray cda[static 1], Chunk chunk);
Chunk *cdaAddChunk(ChunkDynamicArray cda[static 1], ChunkId cid);
Chunk *cdaFind(ChunkDynamicArray cda, ChunkId cid);
void cdaFree(ChunkDynamicArray cda);

#endif //CGOL_CDA_H
