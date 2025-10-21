#include <stdbool.h>

#include "raylib.h"
#include "types.h"

bool cidEquals(const ChunkId c1, const ChunkId c2) {
    return c1.x == c2.x && c1.y == c2.y;
}

Vector2 toVector2(const Vector2i v) {
    return (Vector2){(float) v.x, (float) v.y};
}

Vector2i toVector2i(const Vector2 v) {
    return (Vector2i){(int) v.x, (int) v.y};
}

Vector2i Vector2iAdd(const Vector2i v1, const Vector2i v2) {
    return (Vector2i){v1.x + v2.x, v1.y + v2.y};
}

