/**
 * Extreme Edge Cases
 * CS 241 - Spring 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

/**
 * This function takes two arrays of strings both ending with NULL,
 * determing if the two arrays contain exact the same strings.
 * 
 * @param str_arr1 - The first array of strings.
 * @param str_arr2 - The second array of strings.
 * 
 * @return -1 if two arrays contain at least one differen string;
 *         the number of strings in one array if two arrays contain the same strings.
 */
int str_arr_cmp(char ** str_arr1, char ** str_arr2) {
    int str_count = 0;
    if (str_arr1 == NULL && str_arr2 == NULL) {
        return str_count;
    } else if (str_arr1 == NULL || str_arr2 == NULL) {
        return -1;
    }
    while (*str_arr1 != NULL) {
        if (*str_arr2 == NULL || strcmp(*str_arr1, *str_arr2) != 0) {
            return -1;
        }
        str_count++;
        str_arr1++;
        str_arr2++;
    }
    return (*str_arr2 == NULL) ? str_count : -1;
}

int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    // TODO: Implement me!

    // Unit #0
    char *input_0 = NULL;
    char **output_0 = camelCaser(input_0);
    int str_count_0 = str_arr_cmp(NULL, output_0);
    if (str_count_0 == -1) {return 0;}
    destroy(output_0);

    // Unit #1
    char *input_1 = "hello. welcome to cs241";
    char *expected_1[] = {"hello", NULL};
    char **output_1 = camelCaser(input_1);
    int str_count_1 = str_arr_cmp(expected_1, output_1);
    if (str_count_1 == -1) {return 0;}
    destroy(output_1);

    // Unit 2
    char *input_2 = "      . ";
    char *expected_2[] = {"", NULL};
    char **output_2 = camelCaser(input_2);
    int str_count_2 = str_arr_cmp(expected_2, output_2);
    if (str_count_2 == -1) {return 0;}
    destroy(output_2);

    // Unit #3
    char *input_3 = "       hello  World  . ";
    char *expected_3[] = {"helloWorld", NULL};
    char **output_3 = camelCaser(input_3);
    int str_count_3 = str_arr_cmp(expected_3, output_3);
    if (str_count_3 == -1) {return 0;}
    destroy(output_3);

    // Unit #4
    char *input_4 = "d  .      : , X - AAA bbB + 290 ? ~#$ + ";
    char *expected_4[] = {"d", "", "", "x", "aaaBbb", "290", "", "", "", "", NULL};
    char **output_4 = camelCaser(input_4);
    int str_count_4 = str_arr_cmp(expected_4, output_4);
    if (str_count_4 == -1) {return 0;}
    destroy(output_4);

    // Unit #5
    char *input_5 = " hi no punc marks  000 ";
    char *expected_5[] = {NULL};
    char **output_5 = camelCaser(input_5);
    int str_count_5 = str_arr_cmp(expected_5, output_5);
    if (str_count_5 == -1) {return 0;}
    destroy(output_5);

    // Unit 6
    char *input_6 = "";
    char *expected_6[] = {NULL};
    char **output_6 = camelCaser(input_6);
    int str_count_6 = str_arr_cmp(expected_6, output_6);
    if (str_count_6 == -1) {return 0;}
    destroy(output_6);

    // Unit #7
    char *input_7 = "I'm on cloud wondering lonely. Goodbye KOBE it's nice to know you.";
    char *expected_7[] = {"i", "mOnCloudWonderingLonely", "goodbyeKobeIt", "sNiceToKnowYou", NULL};
    char **output_7 = camelCaser(input_7);
    int str_count_7 = str_arr_cmp(expected_7, output_7);
    if (str_count_7 == -1) {return 0;}
    destroy(output_7);

    // Unit #8
    char *input_8 = " \n . 1 ^2|3[. ..]aaA ,\r BD{ ca\a9au) 8ub\\ 1xy a0b , 9 } . ";
    char *expected_8[] = {"", "1", "2", "3", "", "", "", "", "aaa", "bd", "ca\a9au", "8ub", "1xyA0b", "9", "", NULL};
    char **output_8 = camelCaser(input_8);
    int str_count_8 = str_arr_cmp(expected_8, output_8);
    if (str_count_8 == -1) {return 0;}
    destroy(output_8);

    // Unit #9
    char *input_9 = "3Q\\T / : G \b G\bE ccc\nm NNxXx!xx";
    char *expected_9[] = {"3q", "t", "", "g\bG\beCccMNnxxx", NULL};
    char **output_9 = camelCaser(input_9);
    int str_count_9 = str_arr_cmp(expected_9, output_9);
    if (str_count_9 == -1) {return 0;}
    destroy(output_9);

    // Unit #10
    char *input_10 = "3Q\\T / : G \b\b G ccc\nm `xx";
    char *expected_10[] = {"3q", "t", "", "g\b\bGCccM", NULL};
    char **output_10 = camelCaser(input_10);
    int str_count_10 = str_arr_cmp(expected_10, output_10);
    if (str_count_10 == -1) {return 0;}
    destroy(output_10);

    // Unit #11
    // input_11 = "............................."     -- # of ('.') = 252, plus '\0'
    char *input_11 =  (char *) malloc(253);
    memset(input_11, '.', 252);
    input_11[252] = '\0';
    // expected_11 = {"", "", "", .... "", NULL}      -- # of ("") = 252, plus NULL
    char *expected_11[253];
    for (int i = 0; i < 252; i++) {expected_11[i] = "";}
    expected_11[252] = NULL;
    // Feed test function
    char **output_11 = camelCaser(input_11);
    int str_count_11 = str_arr_cmp(expected_11, output_11);
    if (str_count_11 == -1) {return 0;}
    destroy(output_11);
    // Free input_11
    free(input_11);
    input_11 = NULL;

    // Unit #12
    // input_12 = "aaaaaaaaaaaaaaaa...aaa!"     -- # of ('a') = 252, plus '!' and '\0'
    char *input_12 = (char *) malloc(254);
    memset(input_12, 'a', 252);
    input_12[252] = '!';
    input_12[253] = '\0';
    // expected_12 = {"aaaa...aa", NULL}        -- # of ('a') in string = 252, and NULL
    char *expected_12[2];
    expected_12[0] = (char *) malloc(253);
    memset(expected_12[0], 'a', 252);
    expected_12[0][252] = '\0';
    expected_12[1] = NULL;
    // Feed test function
    char **output_12 = camelCaser(input_12);
    // // Debug log
    // my_log(input_12);
    // print_strs_log(output_12);
    int str_count_12 = str_arr_cmp(expected_12, output_12);
    if (str_count_12 == -1) {return 0;}
    destroy(output_12);
    // free 
    free(input_12);
    input_12 = NULL;
    free(expected_12[0]);
    expected_12[0] = NULL;

    // // Unit #
    // char *input_# = " \n . 1 ^2|3[. ..]aaA , BD{ ca\a9au) 8ub\\ 1xy a0b , 9 } . ";
    // char *expected_#[] = {};
    // char **output_# = camelCaser(input_#);
    // int str_count_# = str_arr_cmp(expected_#, output_#);
    // if (str_count_# == -1) {return 0;}
    // destroy(output_#);

    return 1;
}