#ifndef DEBUG_BLOCK_H
#define DEBUG_BLOCK_H

#include "cglm/cglm.h"
#include "cglm/call.h"

#include "shader.h"

void debugblock_init();
void debugblock_draw(float x, float y, float z, shader_t shader);

#endif