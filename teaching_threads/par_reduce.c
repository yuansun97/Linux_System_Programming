/**
 * Teaching Threads
 * CS 241 - Spring 2020
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct task {
    pthread_t tid;
    int *list;
    reducer funct;
    size_t start_idx;
    size_t end_idx;
    int base_case;
    int result;
} task;

/* You should create a start routine for your threads. */
void *start_routine(void *ptr) {
    task *arg = (task *) ptr;
    int retval = arg->base_case;
    for (size_t i = arg->start_idx; i < arg->end_idx; ++i) {
        retval = arg->funct(retval, arg->list[i]);
    }
    arg->result = retval;
    return NULL;
}

int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */
    if (num_threads > list_len) 
        num_threads = list_len;
    task threads[num_threads];
    size_t sub_len = list_len / num_threads;
    size_t i;

    size_t remainder = list_len % num_threads;

    for (i = 0; i < num_threads; ++i) {
        threads[i].list = list;
        threads[i].funct = reduce_func;
        if (i < remainder) {
            threads[i].start_idx = i * (sub_len + 1);
            threads[i].end_idx = (i + 1) * (sub_len + 1);
        } else {
            threads[i].start_idx = remainder * (sub_len + 1) + (i - remainder) * sub_len;
            threads[i].end_idx = remainder * (sub_len + 1) + (i - remainder + 1) * sub_len;
        }
        threads[i].base_case = base_case;
        // threads[i].result
    }
    
    // if (list_len % num_threads != 0) {
    //     threads[num_threads - 1].end_idx = list_len;
    // }

    for (size_t i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i].tid, NULL, start_routine, &threads[i]);
    }

    int res = base_case;
    for (size_t i = 0; i < num_threads; ++i) {
        pthread_join(threads[i].tid, NULL);
        res = reduce_func(res, threads[i].result);
    }
    
    return res;
}
