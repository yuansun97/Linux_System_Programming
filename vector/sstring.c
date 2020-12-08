/**
 * Vector
 * CS 241 - Spring 2020
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    // Anything you want
    vector *char_vector;
    // char *cstr;
    // copy_constructor_type char_copy_constructor;
    // destructor_type char_destructor;
    // default_constructor_type char_default_constructor;
};

// Return the character vector for debug purpose
vector *get_vec(sstring *this) {
    #ifdef DEBUG
    return this->char_vector;
    #endif
    return NULL;
}

sstring *cstr_to_sstring(const char *input) {
    // your code goes here

    // assert(input);
    // Allocate memory for sstring
    sstring *this = malloc(sizeof(sstring));
    // Create a char vector pointed by char_vector
    this->char_vector = char_vector_create();
    // Push every character in input to the vector, '\0' NOT included
    size_t len = strlen(input);
    size_t idx = 0;
    while (idx < len) {
        vector_push_back(this->char_vector, (void *) input + idx);
        idx++;
    }
    return this;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    size_t len = vector_size(input->char_vector);
    // plus 1 for '\0'
    char *cstr = malloc(len + 1);
    size_t idx = 0;
    while (idx < len) {
        memmove(cstr + idx, vector_get(input->char_vector, idx), 1);
        idx++;
    }
    // Add '\0' to the end
    cstr[len] = '\0';
    return cstr;
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    // void **to_append = vector_begin(addition->char_vector);
    size_t append_size = vector_size(addition->char_vector);
    size_t idx = 0;
    while (idx < append_size) {
        vector_push_back(this->char_vector, vector_get(addition->char_vector, idx));
        idx++;
    }
    return vector_size(this->char_vector);
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    vector *ret = string_vector_create();
    // index records the position one after last delimiter 
    size_t left_idx = 0;
    size_t len = vector_size(this->char_vector);
    // Go through characters in this and check delimiter
    for (size_t curr_idx = 0; curr_idx < len; curr_idx++) {
        if (*(char *) vector_get(this->char_vector, curr_idx) == delimiter) {
            char *buffer = sstring_slice(this, left_idx, curr_idx);
            vector_push_back(ret, buffer);
            free(buffer);
            left_idx = curr_idx + 1;
        }
    }
    // Append the suffix part after last delimiter
    if (left_idx != 0) {
        char *buffer = sstring_slice(this, left_idx, len);
        vector_push_back(ret, buffer);
        free(buffer);
    }
    return ret;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    // Split string to substrings by target 
    // then append all substrings and substitutions in order.
    vector *str_to_append = string_vector_create();
    // Push back the first substring before index offset
    char *buffer = sstring_slice(this, 0, offset);
    vector_push_back(str_to_append, buffer);
    free(buffer);
    buffer = NULL;
    // Store length of original string
    size_t total_len = vector_size(this->char_vector);
    size_t target_len = strlen(target);
    char *slice_to_compare = NULL;
    size_t left_idx = offset;
    size_t curr_idx = offset;
    int target_flag = -1;
    // Go through the sstring and check target
    while (curr_idx <= total_len - target_len) {
        slice_to_compare = sstring_slice(this, curr_idx, curr_idx + target_len);
        // Push back the previous substring and the substitution if a target found
        if (strcmp(slice_to_compare, target) == 0) {
            target_flag = 0;
            buffer = sstring_slice(this, left_idx, curr_idx);
            vector_push_back(str_to_append, buffer);
            vector_push_back(str_to_append, substitution);
            free(buffer);
            buffer = NULL;
            curr_idx += target_len;
            left_idx = curr_idx;
            free(slice_to_compare);
            slice_to_compare = NULL;
            continue;
        }
        free(slice_to_compare);
        slice_to_compare = NULL;
        curr_idx++;
    }
    // Append the end part
    buffer = sstring_slice(this, left_idx, total_len);
    vector_push_back(str_to_append, buffer);
    free(buffer);
    buffer = NULL;

    // Clear the original char_vector and append all the substrings and substitutions
    //  if there is at leat one target
    if (target_flag == 0) {
        vector_clear(this->char_vector);
        for (size_t i = 0; i < vector_size(str_to_append); i++) {
            sstring *tmp = cstr_to_sstring(vector_get(str_to_append, i));
            sstring_append(this, tmp);
            sstring_destroy(tmp);
        }
    }

    vector_destroy(str_to_append);
    return target_flag;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    size_t len = (size_t) end - start;
    // plus 1 for '\0'
    char *slice = malloc(len + 1);
    size_t idx = 0;
    while (start < end) {
        memmove(slice + idx, vector_get(this->char_vector, start), 1);
        start++;
        idx++;
    }
    slice[len] = '\0';
    return slice;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    vector_destroy(this->char_vector);
    this->char_vector = NULL;
    free(this);
    this = NULL;
}
