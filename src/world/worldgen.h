#ifndef WORLDGEN_H
#define WORLDGEN_H

#include "noise/osnoise.h"

#include "cglm/cglm.h"
#include "cglm/call.h"

void worldgenInit(int64_t seed);
int randInRange(int min, int max);
int noiseHeight(ivec2 pos, ivec2 offset);

#endif