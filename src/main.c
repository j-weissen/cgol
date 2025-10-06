#include <stdio.h>

#include "constants.h"
#include "raylib.h"
#include "raymath.h"

#include "main.h"

void updateCamera(Camera2D *camera) {
    static bool isDragging = false;
    static Vector2 drag_start_position = {0, 0};
    static Vector2 target = {0, 0};
    Vector2 mouse_position = GetMousePosition();

    if (!isDragging && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) { // press
        drag_start_position = mouse_position;
        isDragging = true;
    }
    if (isDragging) {
        camera->target = Vector2Add(target, Vector2Subtract(drag_start_position, mouse_position));
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) { // release
            target = camera->target;
            isDragging = false;
        }
    }
}


Vector2i toVector2i(const Vector2 v) {
    return (Vector2i) {(int)v.x, (int)v.y};
}

ChunkId getChunkId(Vector2 position) {
    const double x_scaled = position.x / (CHUNK_WORLD_SIZE*1.0);
    const double y_scaled = position.y / (CHUNK_WORLD_SIZE*1.0);
    return (ChunkId){ floor(x_scaled), floor(y_scaled)};
}

Vector2 getChunkPosition(ChunkId cid) {
    return (Vector2){(float)cid.x*CHUNK_WORLD_SIZE, (float)cid.y*CHUNK_WORLD_SIZE};
}

void drawGridInChunk(ChunkId cid) {
    Vector2 chunk_corner_lu = getChunkPosition(cid);
    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            DrawRectangleLinesEx(
                (Rectangle){
                    chunk_corner_lu.x+(float)x*CELL_WORLD_SIZE,
                    chunk_corner_lu.y+(float)y*CELL_WORLD_SIZE,
                    CELL_WORLD_SIZE, CELL_WORLD_SIZE},GRID_THICKNESS,
                                 RAYWHITE);
        }
    }
}

void drawGridInVisibleChunks(Camera2D camera) {
    const Vector2 corner_lu = GetScreenToWorld2D((Vector2){0,0}, camera);
    const Vector2 corner_rd = GetScreenToWorld2D((Vector2){WINDOW_WIDTH,WINDOW_HEIGHT}, camera);
    const ChunkId chunk_lu = getChunkId(corner_lu);
    const ChunkId chunk_rd = getChunkId(corner_rd);

    for (int y = chunk_lu.y; y <= chunk_rd.y; ++y) {
        for (int x = chunk_lu.x; x <= chunk_rd.x; ++x) {
            drawGridInChunk((ChunkId){ x, y });
            //DrawRectangleLinesEx((Rectangle){x*CHUNK_WORLD_SIZE,y*CHUNK_WORLD_SIZE,CHUNK_WORLD_SIZE,CHUNK_WORLD_SIZE},GRID_THICKNESS,GREEN);
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

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Conways Game of Life");
    Camera2D camera = {.zoom = 1};

    while (!WindowShouldClose()) {
        updateCamera(&camera);

        BeginDrawing();
        BeginMode2D(camera);

        ClearBackground(BLACK);
        drawGridInVisibleChunks(camera);

        EndMode2D();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
