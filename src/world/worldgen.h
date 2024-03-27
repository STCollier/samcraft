#ifndef WORLDGEN_H
#define WORLDGEN_H

#include "noise/osnoise.h"

#include "cglm/cglm.h"
#include "cglm/call.h"

void worldgenInit(int64_t seed);
int randInRange(int min, int max);
int noiseHeight(ivec3 pos, ivec3 offset);

#endif