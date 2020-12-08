/**
 * Password Cracker
 * CS 241 - Spring 2020
 */
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <crypt.h>

#include "cracker2.h"
#include "format.h"
#include "utils.h"
#include "includes/queue.h"

typedef struct task {
    char usr[9];    // <= 8 chars
    char hash[14];  // 13 chars
    char pass[9];   // 8 chars
    int n;          // # of given chars in password
} task;

typedef struct subtask {
    int thread_idx;
    char usr[9];    // <= 8 chars
    char hash[14];  // 13 chars
    char pass[9];   // 8 chars for password
    int n;          // # of given chars in password
    long start_idx;
    long count;
} subtask;

void *crack_password(void *subtask_pool);
int count_chars(char *password);
bool read_flag();
void write_flag();

static bool cracked = false;
// static bool writing = false;
static long total_hashCount = 0;
static double start_time;
static double start_cpu_time;

pthread_barrier_t barrier_task_start;
pthread_barrier_t barrier_task_end;
pthread_barrier_t barrier_result;
pthread_mutex_t mtx;
pthread_mutex_t flag_mtx;
pthread_cond_t cv;

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    queue *subtask_pool[thread_count];
    for (size_t i = 0; i < thread_count; i++) {
        subtask_pool[i] = queue_create(-1);
    }

    /* Read lines, initialize subtasks and push them into queues. */ 
    char *line = NULL;
    size_t size = 0;
    task t;
    long *start_idx = malloc(sizeof(long));
    long *count = malloc(sizeof(long));
    while ( getline(&line, &size, stdin) >= 0 ) {
        sscanf(line, "%s %s %s", t.usr, t.hash, t.pass);
        t.n = count_chars(t.pass);
        int unknown_letter_count = strlen(t.pass) - t.n;
        /* Initialize subtasks */
        for (size_t i = 0; i < thread_count; i++) {
            subtask *subt = malloc(sizeof(subtask));
            getSubrange(unknown_letter_count, thread_count, i + 1, start_idx, count);

            subt->thread_idx = i + 1;
            strcpy(subt->usr, t.usr);
            strcpy(subt->hash, t.hash);
            strcpy(subt->pass, t.pass);
            subt->n = t.n;
            subt->start_idx = *start_idx;
            subt->count = *count;

            queue_push(subtask_pool[i], (void *) subt);
        }
    }
    free(line);
    free(start_idx);
    free(count);

    /* Create worker threads. */
    pthread_barrier_init(&barrier_task_start, NULL, thread_count);
    pthread_barrier_init(&barrier_task_end, NULL, thread_count);
    pthread_barrier_init(&barrier_result, NULL, thread_count);
    pthread_mutex_init(&mtx, NULL);
    pthread_mutex_init(&flag_mtx, NULL);
    pthread_cond_init(&cv, NULL);

    pthread_t tid[thread_count];
    for (size_t i = 0; i < thread_count; i++) {
        queue_push(subtask_pool[i], (void *) NULL);
        pthread_create(&tid[i], NULL, crack_password, subtask_pool[i]);
    }

    /* Join worker threads and destroy subtasks queues. */
    for (size_t i = 0; i < thread_count; i++) {
        pthread_join(tid[i], NULL);
        queue_destroy(subtask_pool[i]);
    }

    pthread_barrier_destroy(&barrier_task_start);
    pthread_barrier_destroy(&barrier_task_end);
    pthread_barrier_destroy(&barrier_result);
    pthread_mutex_destroy(&mtx);
    pthread_mutex_destroy(&flag_mtx);
    pthread_cond_destroy(&cv);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}

void *crack_password(void *subtask_pool) {
    struct crypt_data cdata;
    cdata.initialized = 0;

    subtask *subt = NULL;
    while ( (subt = (subtask *) queue_pull(subtask_pool)) != NULL ) {
        if (subt->thread_idx == 1) { 
            v2_print_start_user(subt->usr);
            start_time = getTime();
            start_cpu_time = getCPUTime();
        }
        int hashCount = 0;
        int result = 2;     
        const char *hashed = NULL;
        setStringPosition(subt->pass + subt->n, subt->start_idx);
        pthread_barrier_wait(&barrier_task_start);
        v2_print_thread_start(subt->thread_idx, subt->usr, subt->start_idx, subt->pass);
        do {
            if (read_flag()) {
                result = 1;
                break;
            }
            hashCount++;
            hashed = crypt_r(subt->pass, "xx", &cdata);
            if ( strcmp(hashed, subt->hash) == 0 ) {
                // Password cracked!
                // lock mutex? [Reader & Writer problem!]
                write_flag(); // cracked = true;
                result = 0;
                double elapsed = getTime() - start_time;
                double total_cpu_time = getCPUTime() - start_cpu_time;
                // lock mutex
                pthread_mutex_lock(&mtx);
                total_hashCount += hashCount;
                pthread_mutex_unlock(&mtx);
                // unlock mutex
                v2_print_thread_result(subt->thread_idx, hashCount, result);
                pthread_barrier_wait(&barrier_result);
                v2_print_summary(subt->usr, subt->pass, total_hashCount, elapsed, total_cpu_time, result);
                // lock
                pthread_mutex_lock(&mtx);
                total_hashCount = 0;
                pthread_mutex_unlock(&mtx);
                // unlock
                break;
            }
            incrementString(subt->pass);
        } while (hashCount < subt->count);

        if (result) {
            // lock mutex
            pthread_mutex_lock(&mtx);
            total_hashCount += hashCount;
            pthread_mutex_unlock(&mtx);
            // unlock mutex
            v2_print_thread_result(subt->thread_idx, hashCount, result);
            pthread_barrier_wait(&barrier_result);
        }

        if (!read_flag() && subt->thread_idx == 1) {
            double elapsed = getTime() - start_time;
            double total_cpu_time = getCPUTime() - start_cpu_time;
            v2_print_summary(subt->usr, subt->pass, total_hashCount, elapsed, total_cpu_time, result);
            pthread_mutex_lock(&mtx);
            total_hashCount = 0;
            pthread_mutex_unlock(&mtx);
        }

        if (result == 0) {
            write_flag(); // cracked = false;
        }

        free(subt);
        pthread_barrier_wait(&barrier_task_end);
    }
    return NULL;
}

/**
 * This function counts the number of given characters in the password,
 *  and set the '.' to 'a'
 * 
 * */
int count_chars(char *password) {
    int count = 0;
	for (size_t i = 0; i < strlen(password); i++) {
		if (password[i] == '.') 
            password[i] = 'a';
        else 
            count++;
	}
	return count;
}


bool read_flag() {
    pthread_mutex_lock(&flag_mtx);
    bool flag = cracked;
    pthread_mutex_unlock(&flag_mtx);
    return flag;
}

void write_flag() {
    pthread_mutex_lock(&flag_mtx);
    cracked = !cracked;
    pthread_mutex_unlock(&flag_mtx);
}

// bool read_flag() {
//     pthread_mutex_lock(&flag_mtx);
//     while (writing) {
//         pthread_cond_wait(&cv, &flag_mtx);
//     }
//     bool ret = cracked;
//     pthread_cond_broadcast(&cv);
//     pthread_mutex_unlock(&flag_mtx);
//     return ret;
// }

// void write_flag() {
//     pthread_mutex_lock(&flag_mtx);
//     writing = true;
//     cracked = !cracked;
//     writing = false;
//     pthread_cond_broadcast(&cv);
//     pthread_mutex_unlock(&flag_mtx);
// }