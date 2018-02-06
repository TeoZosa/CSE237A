// UCSD CSE237A - WI18
// All rights reserved

// Important! DO NOT MODIFY this file. You will not submit this file.
// This file is provided for your convenience to implement an energy-efficient
// LIST scheduler.
// For more details, please see the instructions in the class website.

// scheduler.h: Scheduler for workloads running on two cores

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#define FREQ_CTL_MIN 0
#define FREQ_CTL_MAX 1

typedef struct shared_variable SharedVariable;

// Initialize the scheduler
void init_scheduler(SharedVariable* sv);

// Do scheduling
// * This will internally call select_workload() you need to implement
// * At the end of each workload, this will also call program_body()
//   that you implemented in Section 1.
void schedule();

// Exit scheduling
void exit_scheduler();

#endif
