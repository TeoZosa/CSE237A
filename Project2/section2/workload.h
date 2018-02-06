// UCSD CSE237A - WI18
// All rights reserved

// Important! DO NOT MODIFY this file. You will not submit this file.
// You need to characterize the workloads, which have dependency,
// given with this file.
// For more details, please see the instructions in the class website.

// workload.h: Interface to access workloads for LIST scheduling

#ifndef _WORKLOAD_SECTION2_H_
#define _WORKLOAD_SECTION2_H_

typedef void* (*WorkloadFunc)(void*); // Same to the FuncType of workload_util.h

// 0. Definition of a single workload
#define NULL_TASK -1
typedef struct {
    // An unique index of this workload
    int idx;

    // Three functions of the workload
    WorkloadFunc workload_init;
    WorkloadFunc workload_body;
    WorkloadFunc workload_exit;

    // Next workload
    // If it is NULL_TASK, this workload is one of the last tasks
    // in a scheduling period.
    int successor_idx;
} WorkloadItem;


// ***** The following functions are implemented in workload_graph.c *****

// 1. Get the number of workloads
int get_num_workloads();

// 2. Get a single workload for the workload index
const WorkloadItem* get_workload(int idx);

// 3. Write the task graph of the given workload to "task_graph.pdf" file
// You have to first install graphviz package
void write_workload_graph();

#endif
