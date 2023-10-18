#include "util.h"

int wrap(int x, int length) {
    int remainder = x % length;
    return remainder < 0 ? remainder + length : remainder;
}

// To One Dimension
int _tod(_ivec2 pos, int arrayLen) {
    return pos.x + pos.y * arrayLen;
}