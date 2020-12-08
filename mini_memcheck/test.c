/**
 * Mini Memcheck
 * CS 241 - Spring 2020
 */
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Your tests here using malloc and free
    void *p1 = malloc(30);
    void *p2 = malloc(40);
    void *p3 = realloc(p1, 40);
    free(p1);
    free(p2);
    free(p3);
    return 0;
}
