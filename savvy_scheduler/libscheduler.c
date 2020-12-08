/**
 * Savvy Scheduler
 * CS 241 - Spring 2020
 * 
 * Partner: haonan3
 */
#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_functions.h"

/**
 * The struct to hold the information about a given job
 */
typedef struct _job_info {
    int id;

    /* TODO: Add any other information and bookkeeping you need into this
     * struct. */
    double arrival_time;
    double start_time;
    double run_time;
    double priority;
    int start;
    double new_start_time;
    double remain_time;
    double new_priority;
} job_info;

double waiting_time;
double turnaround_time;
double response_time;
int total;

priqueue_t pqueue;
scheme_t pqueue_scheme;
comparer_t comparision_func;

void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // Put any additional set up code you may need here
    waiting_time = 0;
    turnaround_time = 0;
    response_time = 0;
    total = 0;
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
    // TODO: Implement me!
    if(((job_info *)(((job *)a))->metadata)->arrival_time < (((job_info *)((job *)b)->metadata))->arrival_time) return -1;
    if(((job_info *)(((job *)a))->metadata)->arrival_time > (((job_info *)((job *)b)->metadata))->arrival_time) return 1;
    if(((job_info *)(((job *)a))->metadata)->id < (((job_info *)((job *)b)->metadata))->id) return -1;
    return 1;
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
    // TODO: Implement me!
    if(((job_info *)(((job *)a))->metadata)->new_priority < (((job_info *)((job *)b)->metadata))->new_priority) return -1;
    if(((job_info *)(((job *)a))->metadata)->new_priority > (((job_info *)((job *)b)->metadata))->new_priority) return 1;
    return break_tie(a, b);
}

int comparer_psrtf(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *a_info = ((job *)a)->metadata;
    job_info *b_info = ((job *)b)->metadata;
    double a_remain_time = a_info->remain_time;
    double b_remain_time = b_info->remain_time;

    if(a_remain_time < b_remain_time) return -1;
    if(a_remain_time > b_remain_time) return 1;
    return break_tie(a, b);
}

int comparer_rr(const void *a, const void *b) {
    // TODO: Implement me!
    if(((job_info *)(((job *)a))->metadata)->priority < (((job_info *)((job *)b)->metadata))->priority) return -1;
    if(((job_info *)(((job *)a))->metadata)->priority > (((job_info *)((job *)b)->metadata))->priority) return 1;
    return break_tie(a, b);
}

int comparer_sjf(const void *a, const void *b) {
    // TODO: Implement me!
    if(((job_info *)(((job *)a))->metadata)->run_time < (((job_info *)((job *)b)->metadata))->run_time) return -1;
    if(((job_info *)(((job *)a))->metadata)->run_time > (((job_info *)((job *)b)->metadata))->run_time) return 1;
    return break_tie(a, b);
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    // TODO: Implement me!
    job_info *temp = malloc(sizeof(job_info));
    temp->id = job_number;
    temp->arrival_time = time;
    temp->start_time = 0;
    temp->run_time = sched_data->running_time;
    temp->priority = sched_data->priority;
    temp->new_priority = sched_data->priority;
    temp->start = 0;
    temp->remain_time = temp->run_time;
    temp->new_start_time = 0;
    total ++;
    newjob->metadata = temp;

    priqueue_offer(&pqueue, newjob);
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    // TODO: Implement me!
    if(!job_evicted) {
        job *newjob = priqueue_poll(&pqueue);
        if(!newjob) { return NULL; }

        job_info *newjob_info = newjob->metadata;
        newjob_info->start_time = newjob_info->start ? newjob_info->start_time : time;
        newjob_info->remain_time -= newjob_info->start ? (time - newjob_info->new_start_time) : 0;
        newjob_info->start = 1;

        newjob_info->new_start_time = time;
        return newjob;
    }

    job_info *job_evicted_info = job_evicted->metadata;
    // job_evicted_info->start_time = job_evicted_info->start_time == 0 ? time : job_evicted_info->start_time;
    job_evicted_info->start_time = job_evicted_info->start ? job_evicted_info->start_time : time;
    job_evicted_info->remain_time -= job_evicted_info->start ? (time - job_evicted_info->new_start_time ): 0;
    job_evicted_info->start = 1;

    job_evicted_info->new_start_time = time;
    if(pqueue_scheme == FCFS || pqueue_scheme == PRI || pqueue_scheme == SJF) {
        return job_evicted;
    }

    // job_evicted_info->remain_time = time - job_evicted_info->start_time;
    job_evicted_info->priority += total;
    priqueue_offer(&pqueue, job_evicted);
    job *temp = priqueue_poll(&pqueue);
    job_info *temp_info = temp->metadata;
    temp_info->start_time = temp_info->start ? temp_info->start_time : time;
    temp_info->start = 1;
    temp_info->new_start_time = time;
    return temp;
}

void scheduler_job_finished(job *job_done, double time) {
    // TODO: Implement me!
    job_info *job_done_info = job_done->metadata;
    waiting_time += time - job_done_info->arrival_time - job_done_info->run_time;
    turnaround_time += time - job_done_info->arrival_time;
    response_time += job_done_info->start_time - job_done_info->arrival_time;
    free(job_done_info);
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    // TODO: Implement me!
    return waiting_time/total;
}

double scheduler_average_turnaround_time() {
    // TODO: Implement me!
    return turnaround_time/total;
}

double scheduler_average_response_time() {
    // TODO: Implement me!
    return response_time/total;
}

void scheduler_show_queue() {
    // OPTIONAL: Implement this if you need it!
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}
