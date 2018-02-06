// UCSD CSE237A - WI18

// Important! REPLACE this with your implementation of Part 1
// For more details, please see the instructions in the class website.

// pmu_reader.h: PMU Reading interface

#include "pmu_reader.h"

void reset_counters(void) {
    // Reset all cycle counter and event counters
    asm volatile ("mcr p15, 0, %0, c9, c12, 0\n\t" :: "r"(0x00000017));  
    asm volatile ("mcr p15, 0, %0, c9, c12, 1\n\t" :: "r"(0x8000003f));  
    asm volatile ("mcr p15, 0, %0, c9, c12, 3\n\t" :: "r"(0x8000003f));  
}

unsigned int get_cyclecount(void) {
    unsigned int value;
    asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(value));  
    return value;
}

// Fill your code in this function to read an event counter.
// One of the following numbers will be given to "cnt_index" 
// to specify the counter index:
// (these are defined in pmu_reader.h)
// #define CNT_INST_EXE 0x00
// #define CNT_L1D_ACSS 0x01
// #define CNT_L1D_MISS 0x02
// #define CNT_LLC_ACSS 0x03
// #define CNT_LLC_MISS 0x04
// #define CNT_TLB_MISS 0x05
unsigned int get_single_event(unsigned int cnt_index) {
    unsigned int value = 0;

    // Implement your code here

    return value;
}

