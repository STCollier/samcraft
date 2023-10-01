#include "slidingwindow.h"  

static int chunkIndex(ivec2 pos) {
   return (pos[0] + pos[1] * RENDER_DISTANCE);
}

bool isValidIndex(struct SlidingWindow *self, ivec2 position) {
    ivec2 relativePosition;
    glm_ivec2_copy((ivec2){0, 0}, relativePosition);

    relativePosition[0] = position[0] - self->minPosition[0];
    relativePosition[1] = position[1] - self->minPosition[1];

    return (relativePosition[0] >= 0 && relativePosition[1] >= 0) && 
           (relativePosition[0] < WINDOW_SIZE && relativePosition[1] < WINDOW_SIZE);
}

void setSWValue(struct SlidingWindow *self, ivec2 position, struct Chunk *chunk) {
    if (isValidIndex(self, position)) {
        ivec2 relativeIndex;
        relativeIndex[0] = (position[0] - self->minPosition[0] + self->indexOffset[0]) % WINDOW_SIZE; 
        relativeIndex[1] = (position[1] - self->minPosition[1] + self->indexOffset[1]) % WINDOW_SIZE;
        self->chunks[chunkIndex(relativeIndex)] = *chunk;
    } else {
        fprintf(stderr, "Index out of bounds!\n");
        exit(EXIT_FAILURE);
    }
}

struct Chunk *getSWValue(struct SlidingWindow *self, ivec2 position) {
    if (isValidIndex(self, position)) {
        ivec2 relativeIndex;
        relativeIndex[0] = (position[0] - self->minPosition[0] + self->indexOffset[0]) % WINDOW_SIZE; 
        relativeIndex[1] = (position[1] - self->minPosition[1] + self->indexOffset[1]) % WINDOW_SIZE;
        return &self->chunks[chunkIndex(relativeIndex)];
    } else {
        fprintf(stderr, "Index out of bounds!\n");
        exit(EXIT_FAILURE);
    }
}

void moveSW(struct SlidingWindow *self, ivec2 newPosition) {
   /* if (self->minPosition[0] < 0 || self->minPosition[0] > WINDOW_SIZE) self->minPosition[0] = WINDOW_SIZE / 2;
    if (self->minPosition[1] < 0 || self->minPosition[1] > WINDOW_SIZE) self->minPosition[1] = WINDOW_SIZE / 2;*/

    self->indexOffset[0] += self->minPosition[0] - newPosition[0];  
    self->indexOffset[1] += self->minPosition[1] - newPosition[1];

    self->indexOffset[0] = self->indexOffset[0] < 0 ? (WINDOW_SIZE + 1 - ((-self->indexOffset[0]) % WINDOW_SIZE)) : (self->indexOffset[0] % WINDOW_SIZE);
    self->indexOffset[1] = self->indexOffset[1] < 0 ? (WINDOW_SIZE + 1 - ((-self->indexOffset[1]) % WINDOW_SIZE)) : (self->indexOffset[1] % WINDOW_SIZE);

    self->minPosition[0] = newPosition[0];
    self->minPosition[1] = newPosition[1];
}