#include "main.h"
#include "constants.h"
#include "raylib.h"
#include "raymath.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void updateCamera(Camera2D *camera) {
  static bool isDragging = false;
  static Vector2 drag_start_position = {0, 0};
  static Vector2 target = {0, 0};
  Vector2 mouse_position = GetMousePosition();

  if (!isDragging && IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) { // press
    drag_start_position = mouse_position;
    isDragging = true;
  }
  if (isDragging) {
    camera->target = Vector2Add(
        target, Vector2Subtract(drag_start_position, mouse_position));
    if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) { // release
      target = camera->target;
      isDragging = false;
    }
  }
}

Vector2i toVector2i(const Vector2 v) { return (Vector2i){(int)v.x, (int)v.y}; }

ChunkId getChunkId(Vector2 position) {
  const double x_scaled = position.x / (CHUNK_WORLD_SIZE * 1.0);
  const double y_scaled = position.y / (CHUNK_WORLD_SIZE * 1.0);
  return (ChunkId){floor(x_scaled), floor(y_scaled)};
}

Vector2 getChunkPosition(ChunkId cid) {
  return (Vector2){(float)cid.x * CHUNK_WORLD_SIZE,
                   (float)cid.y * CHUNK_WORLD_SIZE};
}

void drawGridInChunk(ChunkId cid) {
  Vector2 chunk_corner_lu = getChunkPosition(cid);
  for (int y = 0; y < CHUNK_SIZE; ++y) {
    for (int x = 0; x < CHUNK_SIZE; ++x) {
      DrawRectangleLinesEx(
          (Rectangle){chunk_corner_lu.x + (float)x * CELL_WORLD_SIZE,
                      chunk_corner_lu.y + (float)y * CELL_WORLD_SIZE,
                      CELL_WORLD_SIZE, CELL_WORLD_SIZE},
          GRID_THICKNESS, RAYWHITE);
    }
  }
}

void drawGridInVisibleChunks(Camera2D camera) {
  const Vector2 corner_lu = GetScreenToWorld2D((Vector2){0, 0}, camera);
  const Vector2 corner_rd =
      GetScreenToWorld2D((Vector2){WINDOW_WIDTH, WINDOW_HEIGHT}, camera);
  const ChunkId chunk_lu = getChunkId(corner_lu);
  const ChunkId chunk_rd = getChunkId(corner_rd);

  for (int y = chunk_lu.y; y <= chunk_rd.y; ++y) {
    for (int x = chunk_lu.x; x <= chunk_rd.x; ++x) {
      drawGridInChunk((ChunkId){x, y});
    }
  }
}


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

bool cidEquals(ChunkId c1, ChunkId c2) {
  return c1.x == c2.x && c1.y == c2.y;
}
//-----------------------------------------------------------------------------------------------------
#define CDA_INITAL_CAPACITY 32

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
    newCapacity *= 1.5;
  }
  cda->items = realloc(cda->items, newCapacity * sizeof(Chunk *));
  if (cda->items == NULL) {
    fprintf(stderr, "Couldn't allocate, buy more RAM\n");
    exit(1);
  }
  cda->capacity = newCapacity;
}

Chunk *cdaAppend(ChunkDynamicArray cda[static 1], const Chunk chunk) {
  cdaEnsureCapacity(cda, cda->count + 1);
  cda->items[cda->count++] = chunk;
  return &cda->items[cda->count-1];
}

Chunk *cdaFind(ChunkDynamicArray cda, ChunkId cid) {
  // Using a hashmap would be the better solution here. For simplicity, I just used linear search.
  for (int i = 0; i < cda.count; ++i) {
    if (cidEquals(cda.items[i].cid, cid)) {
      return &cda.items[i];
    }
  }
  return NULL;
}

void cdaFree(ChunkDynamicArray cda) {
  for (size_t i = 0; i < cda.count; ++i) {
    free(cda.items[i].cells);
  }
  free(cda.items);
}
//-----------------------------------------------------------------------------------------------------

GlobalCellId getGlobalCellId(Vector2 position) {
  ChunkId chunkId = getChunkId(position);
  Vector2 offsetFromChunkOrigin = Vector2Subtract(position, (Vector2){chunkId.x * CHUNK_WORLD_SIZE, chunkId.y * CHUNK_WORLD_SIZE});
  const double x_scaled = offsetFromChunkOrigin.x / (CELL_WORLD_SIZE * 1.0);
  const double y_scaled = offsetFromChunkOrigin.y / (CELL_WORLD_SIZE * 1.0);
  CellId cellId = (CellId){floor(x_scaled), floor(y_scaled)};
  return (GlobalCellId){chunkId, cellId};
}

size_t cellIdToIndex(CellId cid) {
  return cid.x + cid.y * CHUNK_SIZE;
}

void toggleCellState(ChunkDynamicArray cda[static 1], GlobalCellId gcid) {
  Chunk *clickedChunk = cdaFind(*cda, gcid.chunkId);
  if (clickedChunk == NULL) {
    Cell *cells = malloc(CHUNK_SIZE * CHUNK_SIZE * sizeof(Cell));
    if (cells == NULL) {
      fprintf(stderr, "Couldn't allocate, buy more RAM\n");
      exit(1);
    }
    memset(cells, 0, CHUNK_SIZE * CHUNK_SIZE * sizeof(Cell));
    clickedChunk = cdaAppend(cda, (Chunk){.cid = gcid.chunkId, .cells = cells});
  }
  bool currentCellState = clickedChunk->cells[cellIdToIndex(gcid.cellId)].alive;
  clickedChunk->cells[cellIdToIndex(gcid.cellId)].alive = !currentCellState;
}

Vector2 toVector2(Vector2i v) {
  return (Vector2){(float)v.x, (float)v.y};
}

void drawChunk(Chunk chunk) {
  Vector2 chunkWorldOrigin = Vector2Scale(toVector2(chunk.cid), CHUNK_WORLD_SIZE);
  for (int y = 0; y < CHUNK_SIZE; ++y) {
    for (int x = 0; x < CHUNK_SIZE; ++x) {
      if (chunk.cells[cellIdToIndex((CellId){x,y})].alive) {
        //printf("(%d, %d)\n", x, y);
        DrawRectangleV(Vector2Add(chunkWorldOrigin, (Vector2){x*CELL_WORLD_SIZE, y*CELL_WORLD_SIZE}), (Vector2){CELL_WORLD_SIZE, CELL_WORLD_SIZE}, RAYWHITE);
      }
    }
  }
}

void drawVisibleChunks(Camera2D camera, const ChunkDynamicArray cda) {
  const Vector2 corner_lu = GetScreenToWorld2D((Vector2){0, 0}, camera);
  const Vector2 corner_rd =
      GetScreenToWorld2D((Vector2){WINDOW_WIDTH, WINDOW_HEIGHT}, camera);
  const ChunkId chunk_lu = getChunkId(corner_lu);
  const ChunkId chunk_rd = getChunkId(corner_rd);

  for (int y = chunk_lu.y; y <= chunk_rd.y; ++y) {
    for (int x = chunk_lu.x; x <= chunk_rd.x; ++x) {
      Chunk *chunk = cdaFind(cda, (ChunkId){x,y});
      if (chunk == NULL) continue;
      drawChunk(*chunk);
    }
  }
}

void printGcid(GlobalCellId gcid) {
  printf("Chunk(%d, %d), Cell(%d, %d)", gcid.chunkId.x, gcid.chunkId.y, gcid.cellId.x, gcid.cellId.y);
}

bool getCellStatus(ChunkDynamicArray cda, GlobalCellId gcid) {
  Chunk *chunk = cdaFind(cda, gcid.chunkId);
  if (chunk == NULL) return false; // chunk couldn't be found in the array => must be empty/dead
  return chunk->cells[cellIdToIndex(gcid.cellId)].alive;
}

GlobalCellId applyOffset(GlobalCellId gcid, int dx, int dy) {
  GlobalCellId n = (GlobalCellId){
    .chunkId = gcid.chunkId,
    .cellId = {
      .x = gcid.cellId.x + dx,
      .y = gcid.cellId.y + dy,
    }};
  if (n.cellId.x < 0) {
    n.chunkId.x--;
    n.cellId.x += (CHUNK_SIZE + dx);
  } else if (n.cellId.x >= CHUNK_SIZE) {
    n.chunkId.x++;
    n.cellId.x -= (CHUNK_SIZE + dx);
  }

  if (n.cellId.y < 0) {
    n.chunkId.y--;
    n.cellId.y += (CHUNK_SIZE + dy);
  } else if (n.cellId.y >= CHUNK_SIZE) {
    n.chunkId.y++;
    n.cellId.y -= (CHUNK_SIZE + dy);
  }
  return n;
}

unsigned int countNeighbors(ChunkDynamicArray cda, GlobalCellId gcid) {
  unsigned int neighbors = 0;
  for (int dy = -1; dy <= 1; ++dy) {
    for (int dx = -1; dx <= 1; ++dx) {
      if (dx == 0 && dy == 0) continue;
      GlobalCellId neighbor = applyOffset(gcid, dx, dy);
      if (getCellStatus(cda, neighbor)) {
        neighbors++;
        printGcid(gcid);
        printf(" -> ");
        printGcid(neighbor);
        printf(" -> true\n");
      }
    }
  }
  return neighbors;
}

Cell *getCell(ChunkDynamicArray cda, GlobalCellId gcid) {
  return &cdaFind(cda, gcid.chunkId)->cells[cellIdToIndex(gcid.cellId)];
}

void calculateNextGen(ChunkDynamicArray cda) {
  for (int chunkIndex = 0; chunkIndex < cda.count; ++chunkIndex) {
    for (int y = 0; y < CHUNK_SIZE; ++y) {
      for (int x = 0; x < CHUNK_SIZE; ++x) {
        GlobalCellId gcid = (GlobalCellId){cda.items[chunkIndex].cid, (CellId){x, y}};
        unsigned int neighborsCount = countNeighbors(cda, gcid);
        if (neighborsCount == 3) {
          getCell(cda, gcid)->alive_next_gen = true;
        } else if (neighborsCount == 2 && getCellStatus(cda, gcid)) {
          getCell(cda, gcid)->alive_next_gen = true;
        } else {
          getCell(cda, gcid)->alive_next_gen = false;
        }
        printf("%d", neighborsCount);
      }
      printf("\n");
    }
  }
}

void applyNextGen(ChunkDynamicArray cda) {
  for (int chunkIndex = 0; chunkIndex < cda.count; ++chunkIndex) {
    for (int y = 0; y < CHUNK_SIZE; ++y) {
      for (int x = 0; x < CHUNK_SIZE; ++x) {
        GlobalCellId gcid = (GlobalCellId){cda.items[chunkIndex].cid, (CellId){x, y}};
        Cell *cell = getCell(cda, gcid);
        cell->alive = cell->alive_next_gen;
      }
    }
  }
}

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Conways Game of Life");
  SetTargetFPS(TARGET_FPS);
  Camera2D camera = {.zoom = 1};
  ChunkDynamicArray cda = cdaInit();

  while (!WindowShouldClose()) {
    updateCamera(&camera);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Vector2 screenMousePosition = GetMousePosition();
      Vector2 worldMousePosition = GetScreenToWorld2D(screenMousePosition, camera);
      GlobalCellId clickedGlobalCellId = getGlobalCellId(worldMousePosition);
      printf("Chunk(%d, %d), Cell(%d, %d)\n", clickedGlobalCellId.chunkId.x, clickedGlobalCellId.chunkId.y, clickedGlobalCellId.cellId.x, clickedGlobalCellId.cellId.y);
      toggleCellState(&cda, clickedGlobalCellId);
    }

    if (IsKeyPressed(KEY_SPACE)) {
      calculateNextGen(cda);
      applyNextGen(cda);
    }

    BeginDrawing();
    BeginMode2D(camera);

    ClearBackground(BLACK);
    drawGridInVisibleChunks(camera);
    drawVisibleChunks(camera, cda);

    EndMode2D();
    EndDrawing();
  }

  CloseWindow();
  cdaFree(cda);
  return 0;
}
