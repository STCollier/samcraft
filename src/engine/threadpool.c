#include "threadpool.h"
#include "util.h"

int getCoreCount() {
    int corect;
    #if defined(_WIN32)
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        corect = sysinfo.dwNumberOfProcessors;
    #elif defined(BSD) || defined(__APPLE__)
        corect = 0;
        int mib[4];
        size_t len = sizeof(corect);
        mib[0] = CTL_HW;
        mib[1] = HW_AVAILCPU;
        sysctl(mib, 2, &corect, &len, NULL, 0);
    #else
        corect = sysconf(_SC_NPROCESSORS_ONLN);
    #endif
    if (corect < 1) return 1;
    return corect;
}

static tpool_work_t *tpool_workCreate(thread_func_t func, void *arg) {
    tpool_work_t *work;

    if (func == NULL) return NULL;

    work = malloc(sizeof(*work));
    work->func = func;
    work->arg  = arg;
    work->next = NULL;

    return work;
}

static void tpool_work_destroy(tpool_work_t *work) {
    if (work == NULL) return;
    free(work);
}

static tpool_work_t *tpool_work_get(tpool_t *pool) {
    tpool_work_t *work;

    if (pool == NULL) return NULL;

    work = pool->work_first;
    if (work == NULL) return NULL;

    if (work->next == NULL) {
        pool->work_first = NULL;
        pool->work_last  = NULL;
    } else {
        pool->work_first = work->next;
    }

    return work;
}

static void *tpool_worker(void *arg) {
    tpool_t *pool = arg;
    tpool_work_t *work;

    while (true) {
        pthread_mutex_lock(&pool->work_mutex);

        while (pool->work_first == NULL && !pool->stop) {
            pthread_cond_wait(&(pool->work_cond), &pool->work_mutex);
        }

        if (pool->stop) break;

        work = tpool_work_get(pool);
        pool->working_count++;
        pthread_mutex_unlock(&pool->work_mutex);

        if (work != NULL) {
            work->func(work->arg);
            tpool_work_destroy(work);
        }

        pthread_mutex_lock(&pool->work_mutex);
        pool->working_count--;
        if (!pool->stop && pool->working_count == 0 && pool->work_first == NULL) {
            pthread_cond_signal(&pool->working_cond);
        }

        pthread_mutex_unlock(&pool->work_mutex);
    }

    pool->thread_count--;
    pthread_cond_signal(&pool->working_cond);
    pthread_mutex_unlock(&pool->work_mutex);
    return NULL;
}

tpool_t *tpool_create(size_t numThreads) {
    tpool_t *pool;
    pthread_t thread;
    size_t i;

    if (numThreads == 0) {
        WARN("Invalid value of 0 threads. Defaulting to 2 threads");
        numThreads = 2;
    }

    pool = calloc(1, sizeof(*pool));
    pool->thread_count = numThreads;

    pthread_mutex_init(&pool->work_mutex, NULL);
    pthread_cond_init(&pool->work_cond, NULL);
    pthread_cond_init(&pool->working_cond, NULL);

    pool->work_first = NULL;
    pool->work_last  = NULL;

    for (i = 0; i < numThreads; i++) {
        pthread_create(&thread, NULL, tpool_worker, pool);
        pthread_detach(thread);
    }

    return pool;
}

void tpool_destroy(tpool_t *pool) {
    tpool_work_t *work, *work2;

    if (pool == NULL)
        return;

    pthread_mutex_lock(&pool->work_mutex);
    work = pool->work_first;

    while (work != NULL) {
        work2 = work->next;
        tpool_work_destroy(work);
        work = work2;
    }

    pool->work_first = NULL;
    pool->stop = true;
    pthread_cond_broadcast(&pool->work_cond);
    pthread_mutex_unlock(&pool->work_mutex);

    tpool_wait(pool);

    pthread_mutex_destroy(&pool->work_mutex);
    pthread_cond_destroy(&pool->work_cond);
    pthread_cond_destroy(&pool->working_cond);

    free(pool);
}

bool tpool_addWork(tpool_t *pool, thread_func_t func, void *arg) {
    tpool_work_t *work;

    if (pool == NULL) return false;

    work = tpool_workCreate(func, arg);
    if (work == NULL) return false;

    pthread_mutex_lock(&pool->work_mutex);
    if (pool->work_first == NULL) {
        pool->work_first = work;
        pool->work_last = pool->work_first;
    } else {
        pool->work_last->next = work;
        pool->work_last = work;
    }

    pthread_cond_broadcast(&pool->work_cond);
    pthread_mutex_unlock(&pool->work_mutex);

    return true;
}

void tpool_wait(tpool_t *pool) {
    if (pool == NULL) return;

    pthread_mutex_lock(&pool->work_mutex);
    while (true) {
        if (pool->work_first != NULL || (!pool->stop && pool->working_count != 0) || (pool->stop && pool->thread_count != 0)) {
            pthread_cond_wait(&pool->working_cond, &pool->work_mutex);
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&pool->work_mutex);
}