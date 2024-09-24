#ifndef TYPES_H
#define TYPES_H

#define INHERIT (vec2){-1, -1} // Used for sprites when the dimensions are to be the actual dimensions of the image. vec2 sprite.dimensions should never be less than 0
#define CLEAR_COLOR (ivec4){255, 255, 255, 255}

#ifndef M_PI
    #define M_PI
#endif

// cglm uses arrays for vectors which is quite annyoing since they can't be returned in a function, let alone being unintuitive with [0] and [1]
typedef struct {
    int x;
    int y;
} ivec2s;

typedef struct {
    int x;
    int y;
    int z;
} ivec3s;

typedef enum {
    RIGHT,
    LEFT,
    TOP,
    BOTTOM,
    FRONT,
    BACK
} Direction;

typedef enum {
    NORTH,
    EAST,
    SOUTH,
    WEST
} CardinalDirection;

typedef enum {
    AXIS_X,
    AXIS_Y,
    AXIS_Z
} Axis;

typedef enum {
    ALIGN_CENTER,
    ALIGN_LEFT,
    ALIGN_RIGHT
} AlignMode;

#endif