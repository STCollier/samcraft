#ifndef WORLDGEN_H
#define WORLDGEN_H

#include "../engine/util/common.h"

void worldgenInit(int64_t seed);
int randInRange(int min, int max);
int noiseHeight(ivec3 pos, ivec3 offset);

#endif