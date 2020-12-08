/**
 * Perilous Pointers
 * CS 241 - Spring 2020
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
    first_step(81);

    int second_val = 132;
    second_step(&second_val);

    int third_val = 8942;
    int *third_ptr = &third_val;
    double_step(&third_ptr);

    //     -- -- -- --|-- -- -- --
    // 0x: 01 00 00 00 00 0f 00 00
    //     00 00 00 00
    // fourth_vals starts from 0x
    int fourth_vals[] = {1, 15 * 256, 0};
    strange_step((char *)fourth_vals);

    int fifth_val = 0;
    empty_step(&fifth_val);

    char sixth_arr[] = {'x', 'x', 'x', 'u'};
    two_step((void *) sixth_arr, sixth_arr);

    char seventh[5];
    three_step(seventh, seventh + 2, seventh + 4);

    char eighth[] = {'x', 'a', 'i', 'q'};
    step_step_step(eighth, eighth, eighth);

    int nineth = 1;
    it_may_be_odd((char *) &nineth, nineth);

    char tenth[] = "x,CS241";
    tok_step(tenth);

    // ox: 00 00 02 01
    int eleventh = 513;
    the_end((void *) &eleventh, (void *) &eleventh);

    return 0;
}
