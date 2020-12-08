/**
 * Password Cracker
 * CS 241 - Spring 2020
 */
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <crypt.h>

#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "includes/queue.h"

#define LINE_LENGTH 35

int count_chars(char *password);
void *crack_password(void *idx);

typedef struct task {
    char usr[9];    // <= 8 chars
    char hash[14];  // 13 chars
    char pass[9];   // 8 chars
    int n;          // # of given chars in password
} task;

void print_task(task *t) {
    #ifdef DEBUG
    printf("%s %s %s n=%d len=%ld\n", 
        t->usr, t->hash, t->pass, t->n, strlen(t->pass));
    #endif
}

static queue *task_pool;
static int success = 0;
static int fails = 0;
static pthread_mutex_t *mtx1;
static pthread_mutex_t *mtx2;

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads

    mtx1 = malloc(sizeof(pthread_mutex_t));
    mtx2 = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mtx1, NULL);
    pthread_mutex_init(mtx2, NULL);

    task_pool = queue_create(-1);

    /* Read lines, initialize tasks and push them into queue. */ 
    char *line = NULL;
    size_t size = 0;
    while ( getline(&line, &size, stdin) >= 0 ) {
        task *t = malloc(sizeof(task));
        sscanf(line, "%s %s %s", t->usr, t->hash, t->pass);
        t->n = count_chars(t->pass);
        queue_push(task_pool, (void *) t);
    }
    free(line);

    /* Create worker threads */
    pthread_t tid[thread_count];
    int *tnum[thread_count];
    for (size_t i = 0; i < thread_count; i++) {
        tnum[i] = malloc(sizeof(int));
        *tnum[i] = i + 1;
        pthread_create(&tid[i], NULL, crack_password, tnum[i]);
        queue_push(task_pool, (void *) NULL);       // Add NULL as an ending flag
    }

    for (size_t i = 0; i < thread_count; i++) {
        pthread_join(tid[i], NULL);
        free(tnum[i]);
    }
    v1_print_summary(success, fails);

    /* Single thread */
    // pthread_t tid;
    // pthread_create(&tid, NULL, crack_password, task_pool);
    // pthread_join(tid, NULL);

    pthread_mutex_destroy(mtx1);
    pthread_mutex_destroy(mtx2);
    queue_destroy(task_pool);
    
    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}

/**
 * This function pulls a task from the queue and try to crack it.
 * 
 * */
void *crack_password(void *idx) {
    struct crypt_data cdata;
    cdata.initialized = 0;
    task *t = NULL;
    while ( ( t = (task *) queue_pull(task_pool) ) != NULL ) {
        v1_print_thread_start(*(int *) idx, t->usr);
        int hashCount = 0;
        int result = 1;
        const char *hashed = NULL;
        do {
            hashCount++;
            hashed = crypt_r(t->pass, "xx", &cdata);
            if ( strcmp(hashed, t->hash) == 0 ) {
                // Password cracked!
                result = 0;
                // lock mutex
                pthread_mutex_lock(mtx1);
                success++;
                pthread_mutex_unlock(mtx1);
                // unlock mutex
                v1_print_thread_result( *(int*) idx, t->usr, t->pass, hashCount, getThreadCPUTime(), result );
                break;
            }
        } while ( incrementString(t->pass + t->n) == 1 );

        if (result) {
            // lock mutex
            pthread_mutex_lock(mtx2);
            fails++;
            pthread_mutex_unlock(mtx2);
            // unlock mutex
            v1_print_thread_result( *(int*) idx, t->usr, t->pass, hashCount, getThreadCPUTime(), result );
        }
        free(t);
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
