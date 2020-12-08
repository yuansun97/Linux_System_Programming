/**
 * Extreme Edge Cases
 * CS 241 - Spring 2020
 */
#include "camelCaser.h"


/**
 * This function allocate new heap memory which is twice as 
 * the original memory size of the input pointer pointing to, 
 * and copy evrything from the original heap memory to the new memory.
 * 
 * @param ptr - The pointer pointing to the original size of memory.
 *        curr_size - The original size of the memory pointed by ptr.
 * @return The new pointer pointing to twice the size of memory with original data copied.
 **/
char **double_space(char **ptr, int curr_size) {
    if (ptr == NULL || curr_size <= 0) {return NULL;}
    // int new_size = 2 * curr_size;
    // char **new_ptr = (char **) malloc(new_size);
    // function NOT finished!
    return NULL;
}

/**
 * This function splits an input string at every punctuation mark,
 * and stores evry single sentence as a string in an array of strings.
 * Every element in the array is a sentence without punctuation marks.
 * The last element in the array is NULL.
 * 
 * @param input_str - The long string to split.
 * 
 * @return an array of strings ended with NULL;
 *          NULL if the input_str is NULL.
 **/
char **split_sentences(const char *input_str, int input_len) {
    if (input_str == NULL) {return NULL;}
    int MAX_STR_NUM = input_len;          // max # of sentences
    int MAX_STR_LEN = input_len;          // max # of bytes in a sentence
    char **str_ptr = (char **) malloc(MAX_STR_NUM * sizeof(char *));
    char buffer[MAX_STR_LEN];   // plus 1 for the '\0'
    int str_ct = 0;                 // count of sentences
    int char_ct = 0;                // count of chars in current sentence
    int input_char_idx = 0;         // index of char in the input string
    // Go through every char in input string
    while (input_str[input_char_idx] != '\0') {
        char c = input_str[input_char_idx];
        if (ispunct(c)) {
            str_ptr[str_ct] = strndup(buffer, char_ct);
            str_ct++;
            char_ct = 0;
        } else {
            buffer[char_ct] = c;
            char_ct++;
        }
        input_char_idx++;
    }
    // Truncate the str_ptr to actual size
    char **ret = (char **) malloc((str_ct + 1) * sizeof(char *)); // plus 1 for the NULL
    for (int i = 0; i < str_ct; i++) {
        ret[i] = str_ptr[i];
    }
    ret[str_ct] = NULL;
    free(str_ptr);
    return ret;
}

/**
 * This function takes a single string NOT containing any punctuation marks,
 * returns a pinter pointing to the camelCased string.
 * 
 * @param input_str - The sentence without any punctuation marks in it.
 * 
 * @return A pointer to a string after camelCased. 
 **/
char *single_str_camel_caser(char *input_str, int input_len) {
    if (input_str == NULL) {return NULL;}
    int MAX_STR_LEN = input_len;
    char buffer[MAX_STR_LEN];   // to store the camelCased string
    int buff_char_ct = 0;           // # of chars in the buffer
    int input_char_idx = 0;         // index of char in the input string
    int word_ct = 0;                // # of words been stored in buffer
    // Go through every byte in the input string
    while (input_str[input_char_idx] != '\0') {
        char c = input_str[input_char_idx];
        // In case of letters
        if (isalpha(c)) {
            // Upcast c if c is the first letter of non-first words
            if (word_ct != 0 && isspace(input_str[input_char_idx - 1])) {
                c = toupper(c);
            } else {
                c = tolower(c);
            }
            buffer[buff_char_ct] = c;
            buff_char_ct++;
        }
        // In case of spaces
        else if (isspace(c)) {
            // Increase word count if it's end of a word
            if (input_char_idx != 0 && !(isspace(input_str[input_char_idx - 1]))) {
                    word_ct++;
            }
        }
        // Other
        else {
            buffer[buff_char_ct] = c;
            buff_char_ct++;
        }
        input_char_idx++;
    }
    char *ret = strndup(buffer, buff_char_ct);
    return ret;
}

char **camel_caser(const char *input_str) {
    // TODO: Implement me!
    if (input_str == NULL) {return NULL;}

    // Compute the length of input string
    int INPUT_LEN = 0;
    while (input_str[INPUT_LEN] != '\0') {
        INPUT_LEN++;
    }
    INPUT_LEN++;

    char **splitted_strs = split_sentences(input_str, INPUT_LEN);
    if (splitted_strs == NULL) {return NULL;}

    int str_ct = 0;
    int MAX_STR_NUM = 500;
    char *camelCased_strs[MAX_STR_NUM + 1];
    // camelCase every sentence
    while (splitted_strs[str_ct] != NULL) {
        camelCased_strs[str_ct] = single_str_camel_caser(splitted_strs[str_ct], INPUT_LEN);
        free(splitted_strs[str_ct]);
        splitted_strs[str_ct] = NULL;
        str_ct++;
    }
    free(splitted_strs);
    splitted_strs = NULL;
    // Truncate the str_ptr to actual size
    char **ret = (char **) malloc((str_ct + 1) * sizeof(char *)); // plus 1 for the end NULL
    for (int i = 0; i < str_ct; i++) {
        ret[i] = camelCased_strs[i];
    }
    ret[str_ct] = NULL;
    return ret;
}

void destroy(char **result) {
    // TODO: Implement me!
    if (result == NULL) {return;}
    int str_ct = 0;
    while (result[str_ct] != NULL) {
        free(result[str_ct]);
        result[str_ct] = NULL;
        str_ct++;
    }
    free(result);
    return;
}
