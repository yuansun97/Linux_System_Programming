/**
 * Critical Concurrency
 * CS 241 - Spring 2020
 */
#include "barrier.h"

// typedef struct barrier_t {
//     pthread_mutex_t mtx;
//     pthread_cond_t cv;

//     // Total number of threads
//     unsigned int n_threads;

//     // Increasing or decreasing count
//     unsigned int count;

//     // Keeps track of what usage number of the barrier we are at
//     unsigned int times_used;
// } barrier_t;

// The returns are just for errors if you want to check for them.
int barrier_destroy(barrier_t *barrier) {
    int error = 0;
    pthread_mutex_destroy(&barrier->mtx);
    pthread_cond_destroy(&barrier->cv);
    return error;
}

int barrier_init(barrier_t *barrier, unsigned int num_threads) {
    int error = 0;
    pthread_mutex_init(&barrier->mtx, NULL);
    pthread_cond_init(&barrier->cv, NULL);
    barrier->n_threads = num_threads;
    barrier->count = 0;
    barrier->times_used = 1;
    return error;
}

int barrier_wait(barrier_t *barrier) {
    pthread_mutex_lock(&barrier->mtx);

    while (barrier->times_used != 1) {
        pthread_cond_wait(&barrier->cv, &barrier->mtx);
    }
    barrier->count++;
    
    if (barrier->count == barrier->n_threads) {
        barrier->times_used = 0;
        barrier->count--;
        pthread_cond_broadcast(&barrier->cv);
    }
    else {
        while (barrier->count != barrier->n_threads
                && barrier->times_used == 1) {
            pthread_cond_wait(&barrier->cv, &barrier->mtx);
        }
        barrier->count--;
        if (barrier->count == 0) {
            barrier->times_used = 1;
        }
        pthread_cond_broadcast(&barrier->cv);
    }
    
    pthread_mutex_unlock(&barrier->mtx);
    return 0;
}
