#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "cglm/cglm.h"
#include "cglm/call.h"

void initWorldDB(int seed);
void serializeChunk(ivec2 chunkPos, unsigned char *data);

#endif