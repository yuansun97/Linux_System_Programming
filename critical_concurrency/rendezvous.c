/**
 * Critical Concurrency
 * CS 241 - Spring 2020
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "semamore.h"

static char *quote_A;
static char *quote_B;

// Create some Semaphores!
static Semamore *sem_A;
static Semamore *sem_B;

static void *modifyB_printA();
static void *modifyA_printB();

int main(int argc, char **argv) {
    // Initialize your semaphores
    sem_A = malloc(sizeof(Semamore));
    sem_B = malloc(sizeof(Semamore));
    semm_init(sem_A, 0, 1);
    semm_init(sem_B, 0, 1);

    quote_A = strdup("J!epo(u!lopx!ipx!nboz!pg!zpv!ibwf!fwfs!nfu!Ejkltusb-!cvu!"
                     "zpv!qspcbcmz!lopx!uibu!bssphbodf!jo!dpnqvufs!tdjfodf!jt!"
                     "nfbtvsfe!jo!obop.Ejkltusbt/!.!Bmbo!Lbz");
    quote_B = strdup("Uif!rvftujpo!pg!xifuifs!dpnqvufst!dbo!uijol!jt!mjlf!uif!"
                     "rvftujpo!pg!xifuifs!tvcnbsjoft!dbo!txjn/ぞ!.!Fethfs!X/"
                     "!Ejkltusb");

    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, modifyA_printB, NULL);
    pthread_create(&tid2, NULL, modifyB_printA, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    free(quote_B);
    free(quote_A);

    // Destroy your semaphores
    semm_destroy(sem_A);
    semm_destroy(sem_B);

    return 0;
}

static void *modifyA_printB() {
    int i = 0;
    // sleep(3);
    while (quote_A[i]) {
        usleep(rand() & 15); // randomized slowdowns
        quote_A[i++]--;
    }
    semm_post(sem_A);
    semm_wait(sem_B);
    printf("quote_B:\t%s\n", quote_B);
    return NULL;
}

static void *modifyB_printA() {
    int i = 0;
    while (quote_B[i]) {
        usleep(rand() & 100); // randomized slowdowns
        quote_B[i++]--;
    }
    semm_post(sem_B);
    semm_wait(sem_A);
    printf("quote_A:\t%s\n", quote_A);
    return NULL;
}
