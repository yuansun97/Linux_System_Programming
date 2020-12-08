/**
 * Vector
 * CS 241 - Spring 2020
 */
#include "sstring.h"
#include <assert.h>
#include <string.h>

void print_log(char *message) {
    #ifdef DEBUG
    fprintf(stderr, "%s \n", message);
    #endif
}

void print_char_vec(vector *vec, char *message) {
    #ifdef DEBUG
    // assert(vec);
    fprintf(stderr, "%s\nPrint vector:\n size=%zu capacity=%zu\n[", message, vector_size(vec), vector_capacity(vec));
    void **array = vector_front(vec);
    for (size_t i = 0; i < vector_capacity(vec); i++) {
        if (array[i] == NULL) {
            fprintf(stderr, "(nul) ");
        } else {
            fprintf(stderr,"%c ", *(char *) vector_get(vec, i));
        }
    }
    fprintf(stderr, "]\n\n");
    #endif
}

void print_str_vec(vector *vec, char *message) {
    #ifdef DEBUG
    // assert(vec);
    fprintf(stderr, "%s\nPrint vector:\n size=%zu capacity=%zu\n[\n", message, vector_size(vec), vector_capacity(vec));
    void **array = vector_front(vec);
    for (size_t i = 0; i < vector_capacity(vec); i++) {
        if (array[i] == NULL) {
            fprintf(stderr, "(nul) ");
        } else {
            fprintf(stderr,"%s\n", (char *) vector_get(vec, i));
        }
    }
    fprintf(stderr, "]\n\n");
    #endif
}

int main(int argc, char *argv[]) {
    // TODO create some tests

    char *ctr_1 = "God, I'm codi{}ng...Goodbye Kobe.";
    char *ctr_2 = "UIUC. CS..Go{}od things come fro{}m hard working.{{}}{}";
    // Unit 1.1 cstring to sstring
    sstring *sstr_1 = cstr_to_sstring(ctr_1);
    print_char_vec(get_vec(sstr_1), "test 1.1 -- cstr_1 to sstr_1");
    sstring *sstr_2 = cstr_to_sstring(ctr_2);
    print_char_vec(get_vec(sstr_2), "test 1.1 -- cstr_2 to sstr_2");

    // Unit 1.2 sstring to cstring
    char *test_1 = sstring_to_cstr(sstr_1);
    print_log("test 1.2 -- sstr to ctr");
    assert(!strcmp(test_1, ctr_1));
    print_log(test_1);
    free(test_1);
    test_1 = NULL;

    test_1 = sstring_to_cstr(sstr_2);
    assert(!strcmp(test_1, ctr_2));
    print_log(test_1);
    free(test_1);
    test_1 = NULL;

    // Unit 1.3 append
    sstring_append(sstr_1, sstr_2);
    // print ctr after append
    char *test_4 = sstring_to_cstr(sstr_1);
    print_log("test 1.3 -- cstr of sstr_1 after append");
    print_log(test_4);
    free(test_4);
    test_4 = NULL;

    // Unit 1.5 test slice
    char *test_5 = sstring_slice(sstr_1, 0, 15);
    print_log("\ntest 1.5 -- slice between idx [0 15) ");
    print_log(test_5);
    free(test_5);
    test_5 = NULL;

    // Unit 1.6 test split
    vector *splitted_1 = sstring_split(sstr_1, '.');
    // print_log(sstring_to_cstr(sstr_1));
    print_str_vec(splitted_1, "\ntest 1.6 -- split by '.'");
    vector_destroy(splitted_1);

    // Unit 1.7 test substitution
    assert(sstring_substitute(sstr_1, 0, "{}", "[]") == 0);
    char *test_7 = sstring_to_cstr(sstr_1);
    print_log("test 1.7 -- substitution");
    print_log(test_7);
    free(test_7);
    test_7 = NULL;
    

    sstring_destroy(sstr_2);
    sstring_destroy(sstr_1);
    return 0;
}
