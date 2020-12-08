/**
 * Deepfried dd
 * CS 241 - Spring 2020
 */
#include "format.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

void get_options(int argc, char *argv[], int *fds, unsigned int *options);

static int signaled = 0;

void SIGINT_handler(int signum) {
    signaled = 1;
    return;
}

int main(int argc, char **argv) {

    signal(SIGUSR1, SIGINT_handler);

    int fds[2] = {STDIN_FILENO, STDOUT_FILENO};
    unsigned int options[4] = {512, UINT_MAX, 0, 0};
    // Extract arguments
    get_options(argc, argv, fds, options);
    // printf("-b: %u\n", options[0]);
    // printf("-c: %u\n", options[1]);
    // printf("-p: %u\n", options[2]);
    // printf("-k: %u\n", options[3]);

    unsigned int block_size = options[0];
    unsigned int n_blocks   = options[1];
    unsigned int n_skip_in  = options[2];
    unsigned int n_skip_out = options[3];

    FILE *IN = fdopen(fds[0], "r");
    fseek(IN, block_size * n_skip_in, SEEK_SET);

    FILE *OUT = fdopen(fds[1], "w");
    fseek(OUT, block_size * n_skip_out, SEEK_SET);

/**
 * full_blocks_in: number of full blocks read
 * partial_blocks_in: number of non-full blocks read
 * full_blocks_out: number of full blocks written
 * partial_blocks_out: number of partial blocks written
 * total_bytes_copied: total number of bytes copied
 * time_elapsed: total time (in seconds) spent copying
 */
    size_t full_blocks_in = 0; 
    size_t partial_blocks_in = 0;
    size_t full_blocks_out = 0;
    size_t partial_blocks_out = 0;
    size_t total_bytes_copied = 0;
    double time_elapsed;

    struct timespec start, finish;
    clock_gettime(CLOCK_REALTIME, &start);

    size_t nread = 0;
    size_t nwrite = 0;
    char *buf[block_size];
    for (size_t i = 0; i < n_blocks; i++) {
        if (feof(IN) || signaled) {
            break;
        }
        nread = fread(buf, 1, block_size, IN);
        if (nread == block_size) ++full_blocks_in;
        else if (nread > 0) ++partial_blocks_in;

        nwrite = fwrite(buf, 1, nread, OUT);
        if (nwrite == block_size) ++full_blocks_out;
        else if (nwrite > 0) ++partial_blocks_out;
        
        total_bytes_copied += nwrite;
    }

    clock_gettime(CLOCK_REALTIME, &finish);

    time_elapsed = (double) (finish.tv_sec - start.tv_sec) 
            + (double) (finish.tv_nsec - start.tv_nsec) / 1000000000L;
    
    print_status_report(full_blocks_in, partial_blocks_in,
                        full_blocks_out, partial_blocks_out,
                        total_bytes_copied, time_elapsed);

    fclose(IN);
    fclose(OUT);
    return 0;
}


/**
 * This function extracts arguments from "./dd [options]"
 * 
 * @param argc    -- The arguments count
 * @param argv    -- Array of arguments as strings
 * @param fds     -- file descriptors for input and output
 * @param options -- arguments
 * 
 * */
void get_options(int argc, char *argv[], int *fds, unsigned int *options) {
    int opt;
	while ((opt = getopt(argc, argv, "iobcpk")) != -1) {
		switch (opt)
		{
			case 'i': {
                // Open input file
				// printf("Input file:%s\n", argv[optind]);
                fds[0] = open(argv[optind], O_RDONLY);
                if (fds[0] == -1) {
                    // Open file failed!
                    print_invalid_input(argv[optind]);
                    exit(1);
                }
				break;
			}
			case 'o': {
                // Open output file
				// printf("Output file: %s\n", argv[optind]);
                fds[1] = open(argv[optind], O_WRONLY);
                if (fds[1] == -1) {
                    print_invalid_output(argv[optind]);
                    exit(1);
                } 
				break;
			}
            case 'b': {
                // Get block size
				// printf("file arg:%s\n", argv[optind]);
                options[0] = strtoul(argv[optind], NULL, 10);
				break;
			}
            case 'c': {
                // Get number of blocks
				// printf("file arg:%s\n", argv[optind]);
                options[1] = strtoul(argv[optind], NULL, 10);
				break;
			}
            case 'p': {
				// printf("file arg:%s\n", argv[optind]);
                options[2] = strtoul(argv[optind], NULL, 10);
				break;
			}
            case 'k': {
				// printf("file arg:%s\n", argv[optind]);
                options[3] = strtoul(argv[optind], NULL, 10);
				break;
			}
			default: {
				// print_usage();
				break;
            }
		}
	}
    // return options;
}