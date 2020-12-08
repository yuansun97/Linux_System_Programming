/**
 * Utilities Unleashed
 * CS 241 - Spring 2020
 */
#include "format.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>

/**
 * This function takes in an argument of string,
 * return the index of '=' if the argument is valid,
 * return -1 otherwise.
 * 
 * @param arg -- The input arg to check
 * @return Index of '=', -1 otherwise
 * */
int arg_valid(char *arg) {
    int ret = -1;
    int arg_len = strlen(arg);
    for(int i = 0; i < arg_len; i++) {
        char c = arg[i];
        // Set key length if '=' detected
        if (c == '=') {
            // invalid if '=' is at the start/end or there is more than one '='
            if (i == 0 || i == arg_len - 1 || ret != -1) {break;} 
            // valid otherwise
            else {ret = i;}
        } 
        else if (isalpha(c) || isdigit(c) || c == '_' || c == '%') {
            continue;
        } else {
            ret = -1;
            break;
        }
    }
    return ret;
}

int main(int argc, char *argv[]) {
    // At least three arguments required
    if (argc < 3) {print_env_usage();}

    pid_t child = fork();
    if (child == -1) {
        // fork fails
        print_fork_failed();
    }
    if (child > 0) {
        // I'm the parent
        int status;
        waitpid(child, &status, 0);
    } else if (child == 0) {
        // I'm the child
        int idx = 1;
        while (strcmp(argv[idx], "--") != 0) {
            int equal_mark_idx = arg_valid(argv[idx]);
            if (equal_mark_idx == -1) {
                #ifdef DEBUG
                fprintf(stderr, "equal mark %d\n", equal_mark_idx);
                #endif
                print_env_usage();
            } else {
                if (argv[idx][equal_mark_idx + 1] == '%') {
                    char *ground_val = getenv(argv[idx] + equal_mark_idx + 2);
                    if (ground_val == NULL) {
                        ground_val = "";
                    }
                    argv[idx][equal_mark_idx + 1] = '\0';
                    strcat(argv[idx], ground_val);
                }
                #ifdef DEBUG
                fprintf(stderr, "argv[i]: %s\n", argv[idx]);
                #endif
                putenv(argv[idx]); 
            }
            idx++;
        }
        // check "--"
        if (strcmp(argv[idx], "--") != 0) {
            print_env_usage();
            // will exit in print
        }
        idx++;
        // #ifdef DEBUG
        // fprintf(stderr, "argv[idx] %s\n", argv[idx]);
        // #endif
        execvp(argv[idx], &argv[idx]);
        print_exec_failed();
    }
    return 0;
}
