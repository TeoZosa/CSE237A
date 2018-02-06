// UCSD CSE237A - WI18
// All rights reserved

// Important! DO NOT MODIFY this file. You will not submit this file.
// But, you need to implement the relavant functions in assignment2.c
// to implement your energy-efficient LIST scheduler.
// For more details, please see the instructions in the class website.
#ifndef _SECTION2_H_
#define _SECTION2_H_

#include <stdbool.h>
#include "shared_var.h"

// Task selection result
typedef struct {
	int task_idx;   // Task ID
	int freq;       // FREQ_CTL_MIN(0): Low frequency,
                    // FREQ_CTL_MAX(1): High frequency
} TaskSelection;

// You can characterize the given workloads with the task graph
// in this function to make your scheduling strategies.
// This is called the start part of the program before the scheduling.
// (See main_section2.c)
void learn_workloads(SharedVariable* sv);

// Select a workload index among schedulable workloads.
// This is called by the provided scheduler base (schedule() function.)
// You have to return a TaskSelection structure specifying
// the selected task index and the frequency (FREQ_CTL_MIN or FREQ_CTL_MAX)
// that you want to execute.
TaskSelection select_workload(
        SharedVariable* sv, const int core,
        const int num_workloads,
        const bool* schedulable_workloads, const bool* finished_workloads);

// This function is called before scheduling 16 tasks.
// You may implement some code to evaluate performance and power consumption.
void start_scheduling(SharedVariable* sv);

// This function is called whenever all workloads in the task graph 
// are finished. You may evaluate performance and power consumption 
// of your implementation here.
void finish_scheduling(SharedVariable* sv);

#endif
