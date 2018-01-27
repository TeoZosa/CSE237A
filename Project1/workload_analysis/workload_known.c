// UCSD CSE237A - WI18
// All rights reserved

// Important! DO NOT MODIFY this file. You will not submit this file.
// You need to characterize the workloads given with this file.
// For more details, please see the instructions in the class website.

// workload_known.c: Workload 1 & 2

#include <stdio.h>
#include <stdlib.h>

#include "workload.h"

// Workload 1 : A very naive sort for a small memory
static const int wl1_array_size = 128;
void* workload1_init(void* unused) {
    int i;
    int* wl1_array = (int*)malloc(sizeof(int) * wl1_array_size);
    for (i = 0; i < wl1_array_size; ++i) {
        wl1_array[i] = i;
    }

    printf("Workload 1 is ready.\n");
    return wl1_array;
}

void* workload1_body(void* wl1_array_pt) {
    int* wl1_array = (int*) wl1_array_pt;
    const int iterations = 5000;
    int iter, i, j, tmp;
    for (iter = 0; iter < iterations; ++iter) {
        for (i = 0; i < wl1_array_size; ++i) {
            for (j = i; j < wl1_array_size; ++j) {
                if (wl1_array[i] < wl1_array[j]) {
                    tmp = wl1_array[i];
                    wl1_array[i] = wl1_array[j];
                    wl1_array[j] = tmp;
                }
            }
        }
    }

    return NULL;
}

void* workload1_exit(void* wl1_array_pt) {
    int* wl1_array = (int*) wl1_array_pt;
    free(wl1_array);
    return NULL;
}

// Workload 2 : Random memory access simulation
static const int wl2_array_size = 1024*1024*4;
void* workload2_init(void* unused) {
    int i;
    srand(0); // Random seed initialization 

    int* wl2_array = (int*)malloc(sizeof(int) * wl2_array_size);
    for (i = 0; i < wl2_array_size; ++i) {
        wl2_array[i] = (int)&wl2_array[rand() % wl2_array_size];
    }

    printf("Workload 2 is ready.\n");

    return wl2_array;
}

void* workload2_body(void* wl2_array_pt) {
    int* wl2_array = (int*) wl2_array_pt;
    const int iterations = 30000000;
    int iter;
    int* pt_in_array = &wl2_array[0];
    for (iter = 0; iter < iterations; ++iter) {
        pt_in_array = (int*)*pt_in_array;
    }

    return NULL;
}

void* workload2_exit(void* wl2_array_pt) {
    int* wl2_array = (int*) wl2_array_pt;
    free(wl2_array);
    return NULL;
}
