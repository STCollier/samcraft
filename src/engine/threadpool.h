#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include <stdatomic.h>

#ifdef _WIN32
    #include <windows.h>
#elif defined(BSD) || defined(__APPLE__)
    #include <sys/types.h>
    #include <sys/sysctl.h>
#else
    #include <unistd.h>
#endif


typedef void (*thread_func_t)(void *arg);

struct tpool_work {
    thread_func_t func;
    void *arg;
    struct tpool_work *next;
};

typedef struct tpool_work tpool_work_t;

typedef struct {
    tpool_work_t *work_first;
    tpool_work_t *work_last;
    pthread_mutex_t work_mutex;
    pthread_cond_t work_cond;
    pthread_cond_t working_cond;
    size_t working_count;
    size_t thread_count;
    bool stop;
} tpool_t;

int getCoreCount();

tpool_t *tpool_create(size_t numThreads);
void tpool_destroy(tpool_t *pool);

bool tpool_addWork(tpool_t *pool, thread_func_t func, void *arg);
void tpool_wait(tpool_t *pool);

#endif