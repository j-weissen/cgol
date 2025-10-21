#ifndef CGOL_MAIN_H
#define CGOL_MAIN_H

#include "raylib.h"
#include "types.h"


void step(ChunkDynamicArray cda[static 1]);

void growChunksIfNeeded(ChunkDynamicArray *cda);

void applyNextGen(ChunkDynamicArray cda);

void calculateNextGen(ChunkDynamicArray cda);

Cell *getCell(ChunkDynamicArray cda, GlobalCellId gcid);

unsigned int countNeighbors(ChunkDynamicArray cda, GlobalCellId gcid);

GlobalCellId applyOffset(GlobalCellId gcid, int dx, int dy);

bool getCellStatus(ChunkDynamicArray cda, GlobalCellId gcid);

void drawVisibleChunks(Camera2D camera, ChunkDynamicArray cda);

void drawChunk(Chunk chunk);


#endif //CGOL_MAIN_H
