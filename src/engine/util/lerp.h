#include <math.h>

#ifndef LERP_H
#define LERP_H

enum EASINGS {
    LINEAR,
    EASE_IN_QUAD,
    EASE_OUT_QUAD,
    EASE_IN_OUT_QUAD,
    EASE_IN_CUBIC,
    EASE_OUT_CUBIC,
    EASE_IN_OUT_CUBIC,
    EASE_IN_QUART,
    EASE_OUT_QUART,
    EASE_IN_OUT_QUART,
    EASE_IN_QUINT,
    EASE_OUT_QUINT,
    EASE_IN_OUT_QUINT,
    EASE_IN_SINE,
    EASE_OUT_SINE,
    EASE_IN_OUT_SINE,
    EASE_IN_EXPO,
    EASE_OUT_EXPO,
    EASE_IN_OUT_EXPO,
    EASE_IN_CIRC,
    EASE_OUT_CIRC,
    EASE_IN_OUT_CIRC,
    EASE_IN_BACK,
    EASE_OUT_BACK,
    EASE_IN_OUT_BACK,
    EASE_IN_ELASTIC,
    EASE_OUT_ELASTIC,
    EASE_IN_OUT_ELASTIC,
    EASE_IN_BOUNCE,
    EASE_OUT_BOUNCE,
    EASE_IN_OUT_BOUNCE
};

typedef struct {
    float from, to;
} lerp_values_t;

typedef struct {
	enum EASINGS type;
	lerp_values_t values;
	float time;
} lerp_t;

float lerp_impr(float v0, float v1, float t);
float lerp(float v0, float v1, float t);

float linear(float x);
float easeInQuad(float x);
float easeOutQuad(float x);
float easeInOutQuad(float x);
float easeInCubic(float x);
float easeOutCubic(float x);
float easeInOutCubic(float x);
float easeInQuart(float x);
float easeOutQuart(float x);
float easeInOutQuart(float x);
float easeInQuint(float x);
float easeOutQuint(float x);
float easeInOutQuint(float x);
float easeInSine(float x);
float easeOutSine(float x);
float easeInOutSine(float x);
float easeInExpo(float x);
float easeOutExpo(float x);
float easeInOutExpo(float x);
float easeInCirc(float x);
float easeOutCirc(float x);
float easeInOutCirc(float x);
float easeInBack(float x);
float easeOutBack(float x);
float easeInOutBack(float x);
float easeInElastic(float x);
float easeOutElastic(float x);
float easeInOutElastic(float x);
float easeInBounce(float x);
float easeOutBounce(float x);
float easeInOutBounce(float x);

float lerpTo(lerp_t* l, float step);
float lerpFrom(lerp_t* l, float step);

#endif