#ifndef TIMER_H
#define TIMER_H

#include "common.h"

struct Timer {
    bool ended;
    float value, duration;
};

struct Timer timer_new(float duration);
void timer_update(struct Timer* timer);
void timer_reset(struct Timer* timer);

#endif