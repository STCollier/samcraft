#ifndef UTIL_H
#define UTIL_H

#define ERROR(x) fprintf(stderr, "\x1B[0m%s:%d: \x1B[0;31m[ERROR]\x1B[0m %s\n", __FILE__, __LINE__, x);
#define WARN(x) fprintf(stdout, "\x1B[0m%s:%d: \x1B[0;35m[WARNING]\x1B[0m %s\n", __FILE__, __LINE__, x);
#define LOG(x) fprintf(stdout, "\x1B[0m%s:%d: [LOG] %s\n", __FILE__, __LINE__, x);

#endif