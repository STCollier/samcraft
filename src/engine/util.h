#ifndef UTIL_H
#define UTIL_H

#include "cglm/cglm.h"
#include "cglm/call.h"

// cglm has arrays for vectors which is quite annyoing since they can't be returned in a function, let alone being unintuitive with [0] and [1]
typedef struct {
    int x;
    int y;
} _ivec2;

// Macros
#define ERROR(x) fprintf(stderr, "\x1B[0m%s:%d: \x1B[0;31m[ERROR]\x1B[0m %s\n", __FILE__, __LINE__, x);
#define WARN(x) fprintf(stdout, "\x1B[0m%s:%d: \x1B[0;35m[WARNING]\x1B[0m %s\n", __FILE__, __LINE__, x);
#define LOG(x) fprintf(stdout, "\x1B[0m%s:%d: [LOG] %s\n", __FILE__, __LINE__, x);

#define ERROR_MSG(err, msg) fprintf(stderr, "\x1B[0m%s:%d: \x1B[0;31m[ERROR]\x1B[0m %s %s\n", __FILE__, __LINE__, err, msg);
#define ERROR_IMSG(err, msg) fprintf(stderr, "\x1B[0m%s:%d: \x1B[0;31m[ERROR]\x1B[0m %s %d\n", __FILE__, __LINE__, err, msg);
#define WARN_MSG(err, msg) fprintf(stdout, "\x1B[0m%s:%d: \x1B[0;35m[WARNING]\x1B[0m %s %s\n", __FILE__, __LINE__, err, msg);
#define LOG_MSG(err, msg) fprintf(stdout, "\x1B[0m%s:%d: [LOG] %s %s\n", __FILE__, __LINE__, err, msg);

#define STR(x) #x
#define ALMOST_EQUAL(x, y) (fabs((x) - (y)) < (0.01))
#define RAND(min, max) (rand() % (max + 1 - min) + min)

#define force_inline __attribute__((always_inline))

#endif