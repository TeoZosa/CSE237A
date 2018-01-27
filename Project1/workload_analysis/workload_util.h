// UCSD CSE237A - WI18
// All right reserved

// Important! DO NOT MODIFY this file. You will not submit this file.
// This file is provided for your convenience to characterize workloads.
// For more details, please see the instructions in the class website.

// workload_util.h: Utility for workload characterization

#ifndef _WORKLOAD_UTIL_H_
#define _WORKLOAD_UTIL_H_

// Get current system time
typedef unsigned long long TimeType;
TimeType get_current_time_us();

// Definitions used in workload management utilities
typedef void* (*FuncType)(void*);
#define MAX_CPU_IN_RPI3 4

typedef struct {
    unsigned int is_used;   // 0: unused, 1: used
    unsigned int cc;        // cycle counter 
    unsigned int insts;     // instructions
    unsigned int l1access;  // L1 cache accesses
    unsigned int l1miss;    // L1 cache misses
    unsigned int llcaccess; // LLC accesses
    unsigned int llcmiss;   // LLC misses
    unsigned int iTLBmiss;  // instruction TLB miss
} PerfData;

// Register workload for a specific core
// * This function stores given functions, and runs the init function.
void register_workload(
        int core,                   // e.g., 0
        FuncType workload_init,    // e.g., workload1_init
        FuncType workload_body,    // e.g., workload1_body
        FuncType workload_exit     // e.g., workload1_exit
        );

// Run the body of registred workloads, while collecting PMU events.
// The measured PMU information will be stored to perf_msmts.
// See also report_measurement() in main.c
void run_workloads(PerfData* perf_msmts);

// Unregister workload for a specific core
// * This function runs the exit function
void unregister_workload(int core);

// Unregister workloads for all cores
void unregister_workload_all();

#endif
