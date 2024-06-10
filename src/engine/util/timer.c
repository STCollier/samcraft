#include "../core/window.h"

#include "timer.h"

struct Timer timer_new(float duration) {
    struct Timer timer;

    timer.duration = duration;
    timer.value = 0.0f;
    timer.ended = false;

    return timer;
}

void timer_update(struct Timer* timer) {
    if (timer->value >= timer->duration) {
        timer->value = timer->duration;
        timer->ended = true;
    }

    timer->value += window.dt;
}

void timer_reset(struct Timer* timer) {
    timer->value = 0.0f;
    timer->ended = false;
}