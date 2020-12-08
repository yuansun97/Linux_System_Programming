/**
 * Utilities Unleashed
 * CS 241 - Spring 2020
 */
#define _POSIX_C_SOURCE 199309L
#include "format.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>

// struct timespec {
//     time_t tv_sec;  // seconds
//     long tv_nsec;   // nanoseconds
// };

int main(int argc, char *argv[]) {
    // Check whether arguments are valid
    if (argc < 2) {
        print_time_usage();
    }
    // set clocks for start and finish
    struct timespec start, finish;
    // fork and check
    pid_t child = fork();
    // start the clock since child process starts right after fork
    clock_gettime(CLOCK_REALTIME, &start);
    // check if fork succeeds
    if (child == -1) {print_fork_failed();}
    if (child){
        // I'm the parent
        int status;
        waitpid(child, &status, 0);
        // The child program has returned, so get finish time here.
        clock_gettime(CLOCK_REALTIME, &finish);
        // exit 1 if child terminate failed
        if (status != 0) {exit(1);}
        // calculate the time span
        double seconds = (double) (finish.tv_sec - start.tv_sec) + (double) (finish.tv_nsec - start.tv_nsec) / 1000000000L;
        // display the result
        display_results(argv, seconds);
    } else {
        // I'm the child
        execvp(argv[1], &argv[1]);
        print_exec_failed();
    }
    return 0;
}
