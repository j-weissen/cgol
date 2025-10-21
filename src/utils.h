#ifndef CGOL_UTILS_H
#define CGOL_UTILS_H

#include <stdbool.h>

#include "types.h"
#include "raylib.h"

#define VECTOR2I_UP ((Vector2i){0, -1})
#define VECTOR2I_LEFT ((Vector2i){-1, 0})
#define VECTOR2I_RIGHT ((Vector2i){1, 0})
#define VECTOR2I_DOWN ((Vector2i){0, 1})

bool cidEquals(ChunkId c1, ChunkId c2);
Vector2 toVector2(Vector2i v);
Vector2i vector2iAdd(Vector2i a, Vector2i b);

#endif //CGOL_UTILS_H