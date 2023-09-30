#ifndef SLIDING_WINDOW_H
#define SLIDING_WINDOW_H

#include "cglm/cglm.h"
#include "cglm/call.h"

#include <stdbool.h>
#include <stdio.h>

#include "../world/world.h"
#include "../world/chunk.h"

#define WINDOW_SIZE RENDER_DISTANCE * 2

struct SlidingWindow {
    struct Chunk *chunks;
    ivec2 minPosition;
    ivec2 indexOffset;
};

bool isValidIndex(struct SlidingWindow *self, ivec2 position);
void setSWValue(struct SlidingWindow *self, ivec2 position, struct Chunk *chunk);
struct Chunk *getSWValue(struct SlidingWindow *self, ivec2 position);
void moveSW(struct SlidingWindow *self, ivec2 newPosition);

#endif