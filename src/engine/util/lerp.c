#include "lerp.h"

#define C1 (1.70158)
#define C2 (C1 * 1.525)
#define C3 (C1 + 1)
#define C4 ((2 * M_PI) / 3)
#define C5 ((2 * M_PI) / 4.5)

static float bounceOut(float x) {
    const float n1 = 7.5625;
    const float d1 = 2.75;
    float x2 = x;

    if (x2 < 1 / d1) {
        return n1 * x * x;
    } else if (x2 < 2 / d1) {
        x2 -= 1.5;
        return n1 * (x2 / d1) * x2 + 0.75;
    } else if (x < 2.5 / d1) {
        x2 -= 2.25;
        return n1 * (x2 / d1) * x2 + 0.9375;
    } else {
        x2 -= 2.625;
        return n1 * (x2/ d1) * x2 + 0.984375;
    }
}

// Imprecise method, which does not guarantee v = v1 when t = 1, due to floating-point arithmetic error.
// This method is monotonic. This form may be used when the hardware has a native fused multiply-add instruction.
float lerp_impr(float v0, float v1, float t) {
  return v0 + t * (v1 - v0);
}

// Precise method, which guarantees v = v1 when t = 1. This method is monotonic only when v0 * v1 < 0.
// Lerping between same values might not produce the same value
float lerp(float v0, float v1, float t) {
  return (1 - t) * v0 + t * v1;
}

float lerpTo(lerp_t* l, float step) {
    if (l->time < 1.0) {
        l->time += step;
    }

  switch (l->type) {
      case LINEAR:
          return lerp(l->values.from, l->values.to, linear(l->time));
      case EASE_IN_QUAD:
          return lerp(l->values.from, l->values.to, easeInQuad(l->time));
      case EASE_OUT_QUAD:
          return lerp(l->values.from, l->values.to, easeOutQuad(l->time));
      case EASE_IN_OUT_QUAD:
          return lerp(l->values.from, l->values.to, easeInOutQuad(l->time));
      case EASE_IN_CUBIC:
          return lerp(l->values.from, l->values.to, easeInCubic(l->time));
      case EASE_OUT_CUBIC:
          return lerp(l->values.from, l->values.to, easeOutCubic(l->time));
      case EASE_IN_OUT_CUBIC:
          return lerp(l->values.from, l->values.to, easeInOutCubic(l->time));
      case EASE_IN_QUART:
          return lerp(l->values.from, l->values.to, easeInQuart(l->time));
      case EASE_OUT_QUART:
          return lerp(l->values.from, l->values.to, easeOutQuart(l->time));
      case EASE_IN_OUT_QUART:
          return lerp(l->values.from, l->values.to, easeInOutQuart(l->time));
      case EASE_IN_QUINT:
          return lerp(l->values.from, l->values.to, easeInQuint(l->time));
      case EASE_OUT_QUINT:
          return lerp(l->values.from, l->values.to, easeOutQuint(l->time));
      case EASE_IN_OUT_QUINT:
          return lerp(l->values.from, l->values.to, easeInOutQuint(l->time));
      case EASE_IN_SINE:
          return lerp(l->values.from, l->values.to, easeInSine(l->time));
      case EASE_OUT_SINE:
          return lerp(l->values.from, l->values.to, easeOutSine(l->time));
      case EASE_IN_OUT_SINE:
          return lerp(l->values.from, l->values.to, easeInOutSine(l->time));
      case EASE_IN_EXPO:
          return lerp(l->values.from, l->values.to, easeInExpo(l->time));
      case EASE_OUT_EXPO:
          return lerp(l->values.from, l->values.to, easeOutExpo(l->time));
      case EASE_IN_OUT_EXPO:
          return lerp(l->values.from, l->values.to, easeInOutExpo(l->time));
      case EASE_IN_CIRC:
          return lerp(l->values.from, l->values.to, easeInCirc(l->time));
      case EASE_OUT_CIRC:
          return lerp(l->values.from, l->values.to, easeOutCirc(l->time));
      case EASE_IN_OUT_CIRC:
          return lerp(l->values.from, l->values.to, easeInOutCirc(l->time));
      case EASE_IN_BACK:
          return lerp(l->values.from, l->values.to, easeInBack(l->time));
      case EASE_OUT_BACK:
          return lerp(l->values.from, l->values.to, easeOutBack(l->time));
      case EASE_IN_OUT_BACK:
          return lerp(l->values.from, l->values.to, easeInOutBack(l->time));
      case EASE_IN_ELASTIC:
          return lerp(l->values.from, l->values.to, easeInElastic(l->time));
      case EASE_OUT_ELASTIC:
          return lerp(l->values.from, l->values.to, easeOutElastic(l->time));
      case EASE_IN_OUT_ELASTIC:
          return lerp(l->values.from, l->values.to, easeInOutElastic(l->time));
      case EASE_IN_BOUNCE:
          return lerp(l->values.from, l->values.to, easeInBounce(l->time));
      case EASE_OUT_BOUNCE:
          return lerp(l->values.from, l->values.to, easeOutBounce(l->time));
      case EASE_IN_OUT_BOUNCE:
          return lerp(l->values.from, l->values.to, easeInOutBounce(l->time));
      default:
          // Linear lerp if unknown
          return lerp(l->values.from, l->values.to, linear(l->time));
  }
}

float lerpFrom(lerp_t* l, float step) {
  if (l->time > 0.0) {
      l->time -= step;
  }

  switch (l->type) {
      case LINEAR:
          return lerp(l->values.from, l->values.to, linear(l->time));
      case EASE_IN_QUAD:
          return lerp(l->values.from, l->values.to, easeInQuad(l->time));
      case EASE_OUT_QUAD:
          return lerp(l->values.from, l->values.to, easeOutQuad(l->time));
      case EASE_IN_OUT_QUAD:
          return lerp(l->values.from, l->values.to, easeInOutQuad(l->time));
      case EASE_IN_CUBIC:
          return lerp(l->values.from, l->values.to, easeInCubic(l->time));
      case EASE_OUT_CUBIC:
          return lerp(l->values.from, l->values.to, easeOutCubic(l->time));
      case EASE_IN_OUT_CUBIC:
          return lerp(l->values.from, l->values.to, easeInOutCubic(l->time));
      case EASE_IN_QUART:
          return lerp(l->values.from, l->values.to, easeInQuart(l->time));
      case EASE_OUT_QUART:
          return lerp(l->values.from, l->values.to, easeOutQuart(l->time));
      case EASE_IN_OUT_QUART:
          return lerp(l->values.from, l->values.to, easeInOutQuart(l->time));
      case EASE_IN_QUINT:
          return lerp(l->values.from, l->values.to, easeInQuint(l->time));
      case EASE_OUT_QUINT:
          return lerp(l->values.from, l->values.to, easeOutQuint(l->time));
      case EASE_IN_OUT_QUINT:
          return lerp(l->values.from, l->values.to, easeInOutQuint(l->time));
      case EASE_IN_SINE:
          return lerp(l->values.from, l->values.to, easeInSine(l->time));
      case EASE_OUT_SINE:
          return lerp(l->values.from, l->values.to, easeOutSine(l->time));
      case EASE_IN_OUT_SINE:
          return lerp(l->values.from, l->values.to, easeInOutSine(l->time));
      case EASE_IN_EXPO:
          return lerp(l->values.from, l->values.to, easeInExpo(l->time));
      case EASE_OUT_EXPO:
          return lerp(l->values.from, l->values.to, easeOutExpo(l->time));
      case EASE_IN_OUT_EXPO:
          return lerp(l->values.from, l->values.to, easeInOutExpo(l->time));
      case EASE_IN_CIRC:
          return lerp(l->values.from, l->values.to, easeInCirc(l->time));
      case EASE_OUT_CIRC:
          return lerp(l->values.from, l->values.to, easeOutCirc(l->time));
      case EASE_IN_OUT_CIRC:
          return lerp(l->values.from, l->values.to, easeInOutCirc(l->time));
      case EASE_IN_BACK:
          return lerp(l->values.from, l->values.to, easeInBack(l->time));
      case EASE_OUT_BACK:
          return lerp(l->values.from, l->values.to, easeOutBack(l->time));
      case EASE_IN_OUT_BACK:
          return lerp(l->values.from, l->values.to, easeInOutBack(l->time));
      case EASE_IN_ELASTIC:
          return lerp(l->values.from, l->values.to, easeInElastic(l->time));
      case EASE_OUT_ELASTIC:
          return lerp(l->values.from, l->values.to, easeOutElastic(l->time));
      case EASE_IN_OUT_ELASTIC:
          return lerp(l->values.from, l->values.to, easeInOutElastic(l->time));
      case EASE_IN_BOUNCE:
          return lerp(l->values.from, l->values.to, easeInBounce(l->time));
      case EASE_OUT_BOUNCE:
          return lerp(l->values.from, l->values.to, easeOutBounce(l->time));
      case EASE_IN_OUT_BOUNCE:
          return lerp(l->values.from, l->values.to, easeInOutBounce(l->time));
      default:
          // Linear lerp if unknown
          return lerp(l->values.from, l->values.to, linear(l->time));
    }
}

float linear(float x) {
  return x;
}

float easeInQuad(float x) {
  return x * x;
}

float easeOutQuad(float x) {
  return 1 - (1 - x) * (1 - x);
}

float easeInOutQuad(float x) {
  return x < 0.5 ? 2 * x * x : 1 - pow(-2 * x + 2, 2) / 2;
}

float easeInCubic(float x) {
  return x * x * x;
}

float easeOutCubic(float x) {
  return 1 - pow(1 - x, 3);
}

float easeInOutCubic(float x) {
  return x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
}

float easeInQuart(float x) {
  return x * x * x * x;
}

float easeOutQuart(float x) {
  return 1 - pow(1 - x, 4);
}

float easeInOutQuart(float x) {
  return x < 0.5 ? 8 * x * x * x * x : 1 - pow(-2 * x + 2, 4) / 2;
}

float easeInQuint(float x) {
  return x * x * x * x * x;
}

float easeOutQuint(float x) {
  return 1 - pow(1 - x, 5);
}

float easeInOutQuint(float x) {
  return x < 0.5 ? 16 * x * x * x * x * x : 1 - pow(-2 * x + 2, 5) / 2;
}

float easeInSine(float x) {
  return 1 - cos((x * M_PI) / 2);
}

float easeOutSine(float x) {
  return sin((x * M_PI) / 2);
}

float easeInOutSine(float x) {
  return -(cos(M_PI * x) - 1) / 2;
}

float easeInExpo(float x) {
  return x == 0 ? 0 : pow(2, 10 * x - 10);
}

float easeOutExpo(float x) {
  return x == 1 ? 1 : 1 - pow(2, -10 * x);
}

float easeInOutExpo(float x) {
  return x == 0
      ? 0
      : x == 1
      ? 1
      : x < 0.5
      ? pow(2, 20 * x - 10) / 2
      : (2 - pow(2, -20 * x + 10)) / 2;
}

float easeInCirc(float x) {
  return 1 - sqrt(1 - pow(x, 2));
}

float easeOutCirc(float x) {
  return sqrt(1 - pow(x - 1, 2));
}

float easeInOutCirc(float x) {
  return x < 0.5
      ? (1 - sqrt(1 - pow(2 * x, 2))) / 2
      : (sqrt(1 - pow(-2 * x + 2, 2)) + 1) / 2;
}

float easeInBack(float x) {
  return C3 * x * x * x - C1 * x * x;
}

float easeOutBack(float x) {
  return 1 + C3 * pow(x - 1, 3) + C1 * pow(x - 1, 2);
}

float easeInOutBack(float x) {
  return x < 0.5
      ? (pow(2 * x, 2) * ((C2 + 1) * 2 * x - C2)) / 2
      : (pow(2 * x - 2, 2) * ((C2 + 1) * (x * 2 - 2) + C2) + 2) / 2;
}

float easeInElastic(float x) {
  return x == 0
      ? 0
      : x == 1
      ? 1
      : -pow(2, 10 * x - 10) * sin((x * 10 - 10.75) * C4);
}

float easeOutElastic(float x) {
  return x == 0
      ? 0
      : x == 1
      ? 1
      : pow(2, -10 * x) * sin((x * 10 - 0.75) * C4) + 1;
}

float easeInOutElastic(float x) {
  return x == 0
      ? 0
      : x == 1
      ? 1
      : x < 0.5
      ? -(pow(2, 20 * x - 10) * sin((20 * x - 11.125) * C5)) / 2
      : (pow(2, -20 * x + 10) * sin((20 * x - 11.125) * C5)) / 2 + 1;
}

float easeInBounce(float x) {
  return 1 - bounceOut(1 - x);
}

float easeOutBounce(float x) {
  return bounceOut(x);
}

float easeInOutBounce(float x) {
  return x < 0.5
      ? (1 - bounceOut(1 - 2 * x)) / 2
      : (1 + bounceOut(2 * x - 1)) / 2;
}