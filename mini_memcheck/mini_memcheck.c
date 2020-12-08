/**
 * Mini Memcheck
 * CS 241 - Spring 2020
 */
#include "mini_memcheck.h"
#include <stdio.h>
#include <string.h>

meta_data *head;
size_t total_memory_requested;
size_t total_memory_freed;
size_t invalid_addresses;

// typedef struct _meta_data {
//     size_t request_size;
//     const char *filename;
//     void *instruction;
//     struct _meta_data *next;
// } meta_data;

void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    // return NULL if request size is zero
    if (request_size == 0) {return NULL;}
    // malloc memory for the request and check if malloc succeeds
    void *new_memo_block = malloc(sizeof(meta_data) + request_size);
    if (new_memo_block == NULL) {return NULL;}
    // split the head part to store the malloc information
    void *user_memo_start = new_memo_block + sizeof(meta_data);

    meta_data *meta = (meta_data *) new_memo_block;
    meta->request_size = request_size;
    meta->filename = filename;
    meta->instruction = instruction;
    meta->next = head;

    head = meta;
    total_memory_requested += request_size;
    // return the address where the user memory actually starts
    return user_memo_start; 
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
    if (num_elements == 0 || element_size == 0) {return NULL;}
    void *user_mem_start = (void *) mini_malloc(num_elements * element_size, filename, instruction);
    if (user_mem_start == NULL) {return NULL;}
    memset(user_mem_start, 0, num_elements * element_size);
    return user_mem_start;
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here
    if (payload == NULL) {return mini_malloc(request_size, filename, instruction);}
    if (request_size == 0) {return NULL;}
    
    // look for the meta data for the given pointer
    meta_data *prev = NULL;
    meta_data *iter = head;
    while (iter != NULL) {
        if ((void *) iter + sizeof(meta_data) == payload) {
            // update the total usage
            if (request_size >= iter->request_size) {
                total_memory_requested += (request_size - iter->request_size);
            } else {
                total_memory_freed += (iter->request_size - request_size);
            }
            // update meta data
            iter->request_size = request_size;
            iter->filename = filename;
            iter->instruction = instruction;
            void *new_mem_block = realloc((void *) iter, request_size + sizeof(meta_data));
            // update the linked list node pointers
            if (prev != NULL) {
                prev->next = new_mem_block;
            } else {
                head->next = ((meta_data *) new_mem_block)->next;
            }
            return new_mem_block + sizeof(meta_data);
        }
        prev = iter;
        iter = iter->next;
    }
    // No meata data found in the meta list, so the given pointer is not dynamically allocated. 
    // Thus, it is invalid and cannot be reallocated then.
    invalid_addresses++;
    return NULL;
}

void mini_free(void *payload) {
    // your code here
    if (payload == NULL) {return;}
    // look for the meta data for the given pointer in the list 
    meta_data *prev = NULL;
    meta_data *iter = head;
    while (iter != NULL) {
        if ((void *) iter + sizeof(meta_data) == payload) {
            // update the list node next pointer
            if (prev != NULL) {
                prev->next = iter->next;
            } else {
                head = iter->next;
            }
            meta_data *meta = (meta_data *) iter;
            total_memory_freed += meta->request_size;
            free(iter);
            return;
        }
        prev = iter;
        iter = iter->next;
    }
    // No meata data found in the meta list, so the given pointer is not dynamically allocated. 
    // Thus, it is invalid and cannot be freed then.
    invalid_addresses++;
}
