#ifndef __DIN_PH__
#define __DIN_PH__

#include <pthread.h>
#include <stdbool.h>

typedef struct philosopher {
    int phil_id;
    pthread_t thread_handle;
    int eat_count;
} philosopher;

typedef struct spoon {
    int spoon_id;
    bool is_used;
    philosopher *phil;
    pthread_mutex_t mutex;
    pthread_cond_t cv;
} spoon;

#endif
