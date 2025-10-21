#include "raylib.h"
#include "raymath.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "constants.h"
#include "cda.h"
#include "utils.h"


ChunkId getChunkId(Vector2 position) {
    const double x_scaled = position.x / (CHUNK_WORLD_SIZE * 1.0);
    const double y_scaled = position.y / (CHUNK_WORLD_SIZE * 1.0);
    return (ChunkId){floor(x_scaled), floor(y_scaled)};
}

Vector2 getChunkPosition(ChunkId cid) {
    return (Vector2){
        (float) cid.x * CHUNK_WORLD_SIZE,
        (float) cid.y * CHUNK_WORLD_SIZE
    };
}

void drawGridInChunk(ChunkId cid) {
    Vector2 chunk_corner_lu = getChunkPosition(cid);
    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            DrawRectangleLinesEx(
                (Rectangle){
                    chunk_corner_lu.x + (float) x * CELL_WORLD_SIZE,
                    chunk_corner_lu.y + (float) y * CELL_WORLD_SIZE,
                    CELL_WORLD_SIZE, CELL_WORLD_SIZE
                },
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


GlobalCellId getGlobalCellId(Vector2 position) {
    ChunkId chunkId = getChunkId(position);
    Vector2 offsetFromChunkOrigin = Vector2Subtract(
        position,
        (Vector2){
            (float)chunkId.x * CHUNK_WORLD_SIZE,
            (float)chunkId.y * CHUNK_WORLD_SIZE
        }
    );
    const double x_scaled = offsetFromChunkOrigin.x / (CELL_WORLD_SIZE * 1.0);
    const double y_scaled = offsetFromChunkOrigin.y / (CELL_WORLD_SIZE * 1.0);
    const CellId cellId = (CellId){floor(x_scaled), floor(y_scaled)};
    return (GlobalCellId){chunkId, cellId};
}

size_t cellIdToIndex(CellId cid) { return cid.x + cid.y * CHUNK_SIZE; }

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


void drawChunk(const Chunk chunk) {
    const Vector2 chunkWorldOrigin =
            Vector2Scale(toVector2(chunk.cid), CHUNK_WORLD_SIZE);
    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            if (chunk.cells[cellIdToIndex((CellId){x, y})].alive) {
                DrawRectangleV(
                    Vector2Add(chunkWorldOrigin,
                               (Vector2){x * CELL_WORLD_SIZE, y * CELL_WORLD_SIZE}),
                    (Vector2){CELL_WORLD_SIZE, CELL_WORLD_SIZE}, RAYWHITE);
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
            Chunk *chunk = cdaFind(cda, (ChunkId){x, y});
            if (chunk == NULL)
                continue;
            drawChunk(*chunk);
        }
    }
}

bool getCellStatus(ChunkDynamicArray cda, GlobalCellId gcid) {
    Chunk *chunk = cdaFind(cda, gcid.chunkId);
    if (chunk == NULL)
        return false; // chunk couldn't be found in the array => must be empty/dead
    size_t index = cellIdToIndex(gcid.cellId);
    return chunk->cells[index].alive;
}

GlobalCellId applyOffset(const GlobalCellId gcid, const int dx, const int dy) {
    GlobalCellId n = (GlobalCellId){
        .chunkId = gcid.chunkId,
        .cellId = {
            .x = gcid.cellId.x + dx,
            .y = gcid.cellId.y + dy,
        }
    };
    if (n.cellId.x < 0) {
        n.chunkId.x--;
        n.cellId.x += (CHUNK_SIZE);
    } else if (n.cellId.x >= CHUNK_SIZE) {
        n.chunkId.x++;
        n.cellId.x -= (CHUNK_SIZE);
    }

    if (n.cellId.y < 0) {
        n.chunkId.y--;
        n.cellId.y += (CHUNK_SIZE);
    } else if (n.cellId.y >= CHUNK_SIZE) {
        n.chunkId.y++;
        n.cellId.y -= (CHUNK_SIZE);
    }
    return n;
}

unsigned int countNeighbors(ChunkDynamicArray cda, GlobalCellId gcid) {
    unsigned int neighbors = 0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0)
                continue;
            GlobalCellId neighbor = applyOffset(gcid, dx, dy);
            if (getCellStatus(cda, neighbor)) {
                neighbors++;
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
                GlobalCellId gcid =
                        (GlobalCellId){cda.items[chunkIndex].cid, (CellId){x, y}};
                unsigned int neighborsCount = countNeighbors(cda, gcid);
                if (neighborsCount == 3 || (neighborsCount == 2 && getCellStatus(cda, gcid))) {
                    getCell(cda, gcid)->alive_next_gen = true;
                } else {
                    getCell(cda, gcid)->alive_next_gen = false;
                }
            }
        }
    }
}

void applyNextGen(ChunkDynamicArray cda) {
    for (int chunkIndex = 0; chunkIndex < cda.count; ++chunkIndex) {
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                GlobalCellId gcid =
                        (GlobalCellId){cda.items[chunkIndex].cid, (CellId){x, y}};
                Cell *cell = getCell(cda, gcid);
                cell->alive = cell->alive_next_gen;
            }
        }
    }
}


Vector2i vector2iAdd(Vector2i a, Vector2i b) {
    return (Vector2i){a.x + b.x, a.y + b.y};
}

void growChunksIfNeeded(ChunkDynamicArray *cda) {
    size_t currentChunkCount = cda->count;
    for (int chunkIndex = 0; chunkIndex < currentChunkCount; ++chunkIndex) {
        Chunk currentChunk = cda->items[chunkIndex];
        // up
        ChunkId upChunkId = vector2iAdd(currentChunk.cid, VECTOR2I_UP);
        if (cdaFind(*cda, upChunkId) == NULL) {
            for (size_t x = 0; x < CHUNK_SIZE; x++) {
                if (currentChunk.cells[cellIdToIndex((CellId){x, 0})].alive) {
                    cdaAddChunk(cda, upChunkId);
                    break;
                }
            }
        }
        // down
        ChunkId downChunkId = vector2iAdd(currentChunk.cid, VECTOR2I_DOWN);
        if (cdaFind(*cda, downChunkId) == NULL) {
            for (size_t x = 0; x < CHUNK_SIZE; x++) {
                if (currentChunk.cells[cellIdToIndex((CellId){x, CHUNK_SIZE - 1})]
                    .alive) {
                    cdaAddChunk(cda, downChunkId);
                    break;
                }
            }
        }
        // left
        ChunkId leftChunkId = vector2iAdd(currentChunk.cid, VECTOR2I_LEFT);
        if (cdaFind(*cda, leftChunkId) == NULL) {
            for (size_t y = 0; y < CHUNK_SIZE; y++) {
                if (currentChunk.cells[cellIdToIndex((CellId){0, y})].alive) {
                    cdaAddChunk(cda, leftChunkId);
                    break;
                }
            }
        }
        // right
        ChunkId rightChunkId = vector2iAdd(currentChunk.cid, VECTOR2I_RIGHT);
        if (cdaFind(*cda, rightChunkId) == NULL) {
            for (size_t y = 0; y < CHUNK_SIZE; y++) {
                if (currentChunk.cells[cellIdToIndex((CellId){CHUNK_SIZE - 1, y})].alive) {
                    cdaAddChunk(cda, rightChunkId);
                    break;
                }
            }
        }
    }
}

void step(ChunkDynamicArray cda[static 1]) {
    growChunksIfNeeded(cda);
    calculateNextGen(*cda);
    applyNextGen(*cda);
}


void updateCamera(Camera2D *camera) {
    static bool isDragging = false;
    static Vector2 drag_start_position = {0, 0};
    static Vector2 target = {0, 0};
    Vector2 mouse_position = GetMousePosition();

    if (!isDragging && IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        // press
        drag_start_position = mouse_position;
        isDragging = true;
    }
    if (isDragging) {
        camera->target = Vector2Add(
            target, Vector2Subtract(drag_start_position, mouse_position));
        if (!IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            // release
            target = camera->target;
            isDragging = false;
        }
    }
}

#define CONTROLS_LINE_COUNT 6
const char *controls[] = {
    "Controls",
    "MB right - drag camera",
    "MB left - toggle cell",
    "Space - play/pause",
    "N - step",
    "R - reset"
};

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Conways Game of Life");
    SetTargetFPS(TARGET_FPS);
    Camera2D camera = {.zoom = 1};
    ChunkDynamicArray cda = cdaInit();

    bool isRunning = false;
    float timerInMs = 0.0f;

    while (!WindowShouldClose()) {
        // input
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 screenMousePosition = GetMousePosition();
            Vector2 worldMousePosition =
                    GetScreenToWorld2D(screenMousePosition, camera);
            GlobalCellId clickedGlobalCellId = getGlobalCellId(worldMousePosition);
            toggleCellState(&cda, clickedGlobalCellId);
        }
        if (IsKeyPressed(KEY_N)) {
            step(&cda);
        }
        if (IsKeyPressed(KEY_R)) {
            cdaFree(cda);
            cda = cdaInit();
        }
        if (IsKeyPressed(KEY_SPACE)) {
            isRunning = !isRunning;
        }
        timerInMs += GetFrameTime() * 1000.0f;
        if (isRunning && timerInMs >= STEP_TIME_INTERVAL) {
            step(&cda);
            timerInMs = 0.0f;
        }

        updateCamera(&camera);

        BeginDrawing();
        BeginMode2D(camera);
        ClearBackground(BLACK);
        drawGridInVisibleChunks(camera);
        drawVisibleChunks(camera, cda);
        EndMode2D();
        int maxTextWidth = 0;
        for (size_t i = 0; i < CONTROLS_LINE_COUNT; i++) {
            const int textWidth = MeasureText(controls[i], FONT_SIZE);
            if (textWidth > maxTextWidth) {
                maxTextWidth = textWidth;
            }
        }
        DrawRectangle(FONT_PADDING, FONT_PADDING, maxTextWidth, FONT_SIZE*CONTROLS_LINE_COUNT, BLACK);
        for (size_t i = 0; i < CONTROLS_LINE_COUNT; i++) {
            DrawText(controls[i], FONT_PADDING,FONT_PADDING+i*FONT_SIZE,FONT_SIZE, WHITE);
        }
        EndDrawing();
    }

    CloseWindow();
    cdaFree(cda);
    return 0;
}
