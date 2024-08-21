#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include "../util/common.h"
#include "../core/window.h"
#include "../core/shader.h"

void HDR_init();
void HDR_set();
void HDR_use(shader_t shader);

#endif