/**
 * mapreduce
 * CS 241 - Spring 2020
 */
#include "utils.h" 
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 6) {
        print_usage();
        exit(1);
    }

    char *infile = argv[1];
    char *outfile = argv[2];
    char *mapper = argv[3];
    char *reducer = argv[4];
    int n = atoi(argv[5]);
    // printf("---------------\n");
    // printf("input file:%s;\noutput file:%s\n", infile, outfile);
    // printf("mapper:%s;\nreducer:%s\n", mapper, reducer);
    // printf("# of chunks: %d\n", n);
    // printf("---------------\n");

    // Create an input pipe for each mapper.
    int mapper_fds[n][2];
    for (int i = 0; i < n; i++) {
        pipe2(mapper_fds[i], O_CLOEXEC);
        descriptors_add(mapper_fds[i][0]);
        descriptors_add(mapper_fds[i][1]);
    }

    // Create one input pipe for the reducer.
    int reducer_fds[2];
    pipe2(reducer_fds, O_CLOEXEC);
    descriptors_add(reducer_fds[0]);
    descriptors_add(reducer_fds[1]);

    // Open the output file.
    int fd = open(outfile, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        perror("Output file open failed!\n");
        exit(1);
    }
    descriptors_add(fd);

    // Start a splitter process for each mapper.
    pid_t split_pids[n];
    for (int i = 0; i < n; i++) {
        split_pids[i] = fork();
        if (split_pids[i] == 0) {
            // I'm the splitter child process
            // close(mapper_fds[i][0]);
            // printf("splitting... i=%d\n", i);
            dup2(mapper_fds[i][1], 1);  // Write to mapper pipe
            char istr[5];
            sprintf(istr, "%d", i);
            // printf("***i=%d; istr=%s\n", i, istr);
            execlp("./splitter", "./splitter", infile, argv[5], istr, (char *) NULL);
            perror("Splitter exec Failed!\n");
            exit(1);
        }
    }
    // printf("Splitters done!\n");

    // Start all the mapper processes.
    pid_t map_pids[n];
    for (int i = 0; i < n; i++) {
        map_pids[i] = fork();
        if (map_pids[i] == 0) {
            // I'm the mapper child process
            dup2(mapper_fds[i][0], 0);  // Read from mapper pipe
            dup2(reducer_fds[1], 1);    // Write to reducer pipe
            execlp(mapper, mapper, (char *) NULL);
            perror("Mapper exec Failed!\n");
            exit(1);
        }
    }
    // printf("Mappers done!\n");

    // Start the reducer process.
    pid_t child = fork();
    if (child == 0) {
        // I'm the reducer child process
        dup2(reducer_fds[0], 0);    // Read from reducer pipe
        dup2(fd, 1);           // Write to the output file
        execlp(reducer, reducer, (char *) NULL);
        perror("Reducer exec Failed!\n");
        exit(1);
    }
    // printf("Reducer done!\n");

    descriptors_closeall();
    descriptors_destroy();

    // Wait for the reducer to finish.
    int status = 0;
    waitpid(child, &status, 0);
    // printf("Wait done!\n");

    // Print nonzero subprocess exit codes.
    if (WIFEXITED(status)) {
        if (WEXITSTATUS(status) != 0) {
            print_nonzero_exit_status(reducer, status);
        }
    }

    int status1, status2;
    for (int i = 0; i < n; i++) {
        waitpid(split_pids[i], &status1, 0);
        if (WIFEXITED(status1)) {
            if (WEXITSTATUS(status1) != 0) {
                print_nonzero_exit_status("splitter", status1);
            }
        }

        waitpid(map_pids[i], &status2, 0);
        if (WIFEXITED(status2)) {
            if (WEXITSTATUS(status2) != 0) {
                print_nonzero_exit_status(mapper, status2);
            }
        }
    }

    // Count the number of lines in the output file.
    print_num_lines(outfile);


    return 0;
}
