/**
 * Parallel Make
 * CS 241 - Spring 2020
 */

#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"

int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!

    /* Build the RAG graph. */
    graph *RAG = parser_parse_makefile(makefile, targets);

    return 0;
}
