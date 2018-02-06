// UCSD CSE237A - WI18
// All rights reserved

// Important! DO NOT MODIFY this file. You will not submit this file.
// This file is provided for your convenience to characterize workloads.
// For more details, please see the instructions in the class website.

// workload_util.c: Workload utility 

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <time.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "workload_util.h"
#include "pmu_reader.h"

// Get the current system time
TimeType get_current_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    TimeType curTime = (TimeType)tv.tv_sec * 1000 * 1000 + (TimeType)tv.tv_usec;
    return curTime;
}

// Workload managements ////////////////////////////////////////////////////////
static int core_number[MAX_CPU_IN_RPI3]; // Store <0, 1, 2, 3>
static PerfData perf_msmts_internal[MAX_CPU_IN_RPI3]; // PMU measurements 
static FuncType init_funcs[MAX_CPU_IN_RPI3]; // init function for each core 
static FuncType body_funcs[MAX_CPU_IN_RPI3]; // body function for each core 
static FuncType exit_funcs[MAX_CPU_IN_RPI3]; // exit function for each core 
static void*    work_rptrs[MAX_CPU_IN_RPI3]; // to keep return of init funcs

// Execute the body function of the workload, while collecting PMU events
// The core index is pass to `data' parameter.
static void run_and_profile(void* data) {
    int core = (int) data;

    // 1. Reset PMU counters
    reset_counters(); 
    // 2. Run the actual function
    body_funcs[core](work_rptrs[core]);
    // 3. Read PMU information
    perf_msmts_internal[core].is_used = 1;
    perf_msmts_internal[core].cc = get_cyclecount();
    perf_msmts_internal[core].insts = get_single_event(CNT_INST_EXE);
    perf_msmts_internal[core].l1access = get_single_event(CNT_L1D_ACSS);
    perf_msmts_internal[core].l1miss = get_single_event(CNT_L1D_MISS);
    perf_msmts_internal[core].llcaccess = get_single_event(CNT_LLC_ACSS);
    perf_msmts_internal[core].llcmiss = get_single_event(CNT_LLC_MISS);
    perf_msmts_internal[core].iTLBmiss = get_single_event(CNT_TLB_MISS);
}

// Register workload for a specific core
void register_workload(
        int core,
        FuncType workload_init,
        FuncType workload_body,
        FuncType workload_exit) {
    // Sanity check
    assert(core >= 0);
    assert(core < MAX_CPU_IN_RPI3);

    unregister_workload(core);
    
    init_funcs[core] = workload_init;
    body_funcs[core] = workload_body;
    exit_funcs[core] = workload_exit;

    // Run initialization function and wait
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(core, &cpus);

    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
    pthread_create(&thread, &attr, (void*)init_funcs[core], NULL);
    pthread_join(thread, &work_rptrs[core]);
}

// Run the body of registred workloads, while collecting PMU events.
// The measured PMU information will be stored to perf_msmts.
void run_workloads(PerfData* perf_msmts) {
    int numCPUs = sysconf(_SC_NPROCESSORS_ONLN);
    assert(numCPUs == MAX_CPU_IN_RPI3);

    pthread_t threads[MAX_CPU_IN_RPI3];

    int core;
    // Prepare core_number to pass vias pthread_create
    for (core = 0; core < MAX_CPU_IN_RPI3; ++core) { 
        core_number[core] = core;
    }

    // Run the threads of each workload body on a specified core
    for (core = 0; core < MAX_CPU_IN_RPI3; ++core) {
        if (body_funcs[core] == NULL)
            continue;

        // Set CPU affinity - restrict the working core
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        cpu_set_t cpus;
        CPU_ZERO(&cpus);
        CPU_SET(core, &cpus);

        // Create the thread
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
        pthread_create(
                &threads[core], &attr,
                (void*)run_and_profile, (void*)core_number[core]);
    }

    // Wait until all workloads are finished
    for (core = 0; core < MAX_CPU_IN_RPI3; ++core) {
        if (body_funcs[core] == NULL) {
            perf_msmts[core].is_used = 0;
            continue;
        }
        pthread_join(threads[core], NULL);
        
        // Copy PMU measurement
        memcpy(&perf_msmts[core], &perf_msmts_internal[core], sizeof(PerfData));
    }
}

// Unregister workload for a specific core
void unregister_workload(int core) {
    if (exit_funcs[core] != NULL)
        exit_funcs[core](work_rptrs[core]);

    init_funcs[core] = NULL;
    body_funcs[core] = NULL;
    exit_funcs[core] = NULL;
}

// Unregister workloads for all cores
void unregister_workload_all() {
    int core;
    for (core = 0; core < MAX_CPU_IN_RPI3; ++core) {
        unregister_workload(core);
    }
}

