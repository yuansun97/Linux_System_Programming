/**
 * Vector
 * CS 241 - Spring 2020
 */
#include "vector.h"
#include <stdio.h>
#include <assert.h>

void print_log(char *message, int n) {
    #ifdef DEBUG
    fprintf(stderr, "%s %d ", message, n);
    #endif
}

void print_int_vec(vector *vec, char *message) {
    #ifdef DEBUG
    assert(vec);
    fprintf(stderr, "%s\nPrint vector:\n size=%zu capacity=%zu\n[", message, vector_size(vec), vector_capacity(vec));
    void **array = vector_front(vec);
    for (size_t i = 0; i < vector_capacity(vec); i++) {
        if (array[i] == NULL) {
            fprintf(stderr, "(nul) ");
        } else {
            fprintf(stderr,"%d ", *(int *) vector_get(vec, i));
        }
    }
    fprintf(stderr, "]\n\n");
    #endif
}


int main(int argc, char *argv[]) {
    // Write your test cases here

    // Test create and destroy
    // Test 1 -- shallow create
    vector *vec_1 = vector_create(NULL, NULL, NULL);
    print_int_vec(vec_1, "vec_1");
    vector_destroy(vec_1);

    // Test 2 -- int create
    vector *vec_2 = int_vector_create();
    print_int_vec(vec_2, "vec_2");
    // Unit 2.1 -- resize to smaller
    vector_resize(vec_2, 4);
    print_int_vec(vec_2, "test 2.1 -- Resize 4 ");
    // Unit 2.2 -- set value
    int n_1 = 1;
    vector_set(vec_2, 0, &n_1);
    n_1++;
    vector_set(vec_2, 1, &(n_1));
    n_1++;
    vector_set(vec_2, 2, &(n_1));
    n_1++;
    vector_set(vec_2, 3, &(n_1));
    print_int_vec(vec_2, "test 2.2 -- Set value ");
    // Unit 2.3 -- resize to bigger
    vector_resize(vec_2, 13);
    print_int_vec(vec_2, "test 2.3 -- Resize 13 ");
    // Unit 2.4 -- set value
    int n_2 = 5;
    for (int i = 4; i < 13; i++) {
        vector_set(vec_2, i, &n_2);
        n_2++;
    }
    print_int_vec(vec_2, "test 2.4 -- Set values ");
    // Unit 2.5 -- reverse small n
    vector_reserve(vec_2, 4);
    print_int_vec(vec_2, "test 2.5 -- Reverse 4 ");
    vector_reserve(vec_2, 7);
    print_int_vec(vec_2, "test 2.5 -- Reverse 7 ");
    // Unit 2.6 -- reverse large n
    vector_reserve(vec_2, 15);
    print_int_vec(vec_2, "test 2.6 -- Reverse 15 ");
    // Unit 2.7 -- push back
    for (size_t i = 0; i < 10; i++) {
        n_1++;
        vector_push_back(vec_2, &n_1);
    }
    print_int_vec(vec_2, "test 2.7 -- Push back 10 times ");
    // Unit 2.8 -- pop back
    for (size_t i = 0; i < 5; i++) {
        vector_pop_back(vec_2);
    }
    print_int_vec(vec_2, "test 2.8 -- Pop back 5 times ");
    // Unit 2.9 -- insert
    int INSERT = 99;
    for (size_t i = 0; i < 12; i+=2) {
        // print_log("i", i);
        vector_insert(vec_2, i, &INSERT);
        // print_int_vec(vec_2, "test 2.9 -- Insert even index ");
    }
    print_int_vec(vec_2, "test 2.9 -- Insert even index ");

    // Unit 2.10 -- erase
    for (size_t i = 0; i < 3; i++) {
        // print_log("i", i);
        vector_erase(vec_2, 0);
    }
    print_int_vec(vec_2, "test 2.10 -- Erase idx 0 for 3 times");
    // Unit 2.11 -- clear
    vector_clear(vec_2);
    print_int_vec(vec_2, "test 2.11 -- Clear all");

    vector_destroy(vec_2);
    // Unit test 2
    // Unit test 3

    return 0;
}
