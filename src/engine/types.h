#ifndef TYPES_H
#define TYPES_H

// cglm uses arrays for vectors which is quite annyoing since they can't be returned in a function, let alone being unintuitive with [0] and [1]
typedef struct {
    int x;
    int y;
} _ivec2;

typedef enum {
    RIGHT,
    LEFT,
    FRONT,
    BACK,
    TOP,
    BOTTOM
} Direction;

typedef enum {
    NORTH,
    EAST,
    SOUTH,
    WEST
} CardinalDirection;



#endif