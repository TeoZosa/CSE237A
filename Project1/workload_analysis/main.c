// UCSD CSE237A - WI18
// Important! You need to modify this file, but WILL NOT SUBMIT this file.
// You can characterize the given workloads by chainging this file,
// and WILL SUBMIT the analysis report based on the characterization results.
// For more details, please see the instructions in the class website.

// main.c: Main file to characterize different workloads

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cpufreq.h"
#include "workload_util.h"
#include "workload.h"

#define REGISTER_WL1_CORE_0    register_workload(0, workload1_init, workload1_body, workload1_exit);
#define REGISTER_WL1_CORE_1    register_workload(1, workload1_init, workload1_body, workload1_exit);

#define REGISTER_WL2_CORE_0    register_workload(0, workload2_init, workload2_body, workload2_exit);
#define REGISTER_WL2_CORE_1    register_workload(1, workload2_init, workload2_body, workload2_exit);

#define REGISTER_WL3_CORE_0    register_workload(0, workload3_init, workload3_body, workload3_exit);
#define REGISTER_WL3_CORE_1    register_workload(1, workload3_init, workload3_body, workload3_exit);

#define TRUE            1
#define FALSE           0



// The function can be called after finishing workload(s)
void report_measurement(int freq, PerfData* perf_msmts) {
    int core;
    for (core = 0; core < MAX_CPU_IN_RPI3; ++core) {
        PerfData* pf = &perf_msmts[core]; 
        if (pf->is_used == 0)
            continue;

        TimeType time_estimated = (TimeType)pf->cc/(TimeType)(freq/1000);
        printf("[Core %d] Execution Time (us): %lld\n", core, time_estimated);

        printf("[Core %d] Cycle Count: %u\n", core, pf->cc);
        printf("[Core %d] Instructions: %u\n", core, pf->insts);

        printf("[Core %d] L1 Cache Accesses: %u\n", core, pf->l1access);
        printf("[Core %d] L1 Cache Misses: %u\n", core, pf->l1miss);
        if (pf->l1access != 0)
            printf("[Core %d] L1 Miss Ratio: %lf\n",
                    core, (double)pf->l1miss/(double)pf->l1access);

        printf("[Core %d] LLC Accesses: %u\n", core, pf->llcaccess);
        printf("[Core %d] LLC Misses: %u\n", core, pf->llcmiss);
        if (pf->llcaccess != 0)
            printf("[Core %d] LLC Miss Ratio: %lf\n",
                    core, (double)pf->llcmiss/(double)pf->llcaccess);

        printf("[Core %d] iTLB Misses: %u\n", core, pf->iTLBmiss);
    }
}


int set_CPU_freq(int isMax){
    set_userspace_governor();
    if (isMax){
        set_by_max_freq();
    }else{
        set_by_min_freq();
    }
    return get_cur_freq();
}

PerfData* init_performance_measurements(){
    printf("Characterization starts.\n");
//    PerfData* perf_msmts[MAX_CPU_IN_RPI3];
    PerfData* perf_msmts = malloc(MAX_CPU_IN_RPI3 * sizeof(*perf_msmts));
    return perf_msmts;
}

TimeType run_workload_timed(PerfData* perf_msmts){
    TimeType start_time = get_current_time_us();
    run_workloads(perf_msmts);
    return start_time;
}

void report_perf_msmts(PerfData* perf_msmts, int freq, TimeType start_time){
    printf("Total Execution time (us): %lld at %d\n",
           get_current_time_us() - start_time, get_cur_freq());
    report_measurement(freq, perf_msmts);
}



void cleanup(){
    unregister_workload_all();
    set_ondemand_governor();
}

void run_test(int isMax){
    // 1. Set CPU frequency
    int freq = set_CPU_freq(isMax);
    // 2. Run workload
    printf("Characterization starts.\n");
    PerfData perf_msmts[MAX_CPU_IN_RPI3];
    TimeType start_time = run_workload_timed(perf_msmts);
    // 3. Here, we get elapsed time and performance counters.
    report_perf_msmts(perf_msmts, freq, start_time);
    // 4. Finish the program
    cleanup();
}



void single_core_tests(int isMax){
            // Initialize the workload(s)
    //1-3: One workload at a time on core 0

    printf("WL1 Single Core. \nMax CPU Freq: %d.\n", isMax);
    REGISTER_WL1_CORE_0
    run_test(isMax);

    printf("WL2 Single Core. \nMax CPU Freq: %d.\n", isMax);
    REGISTER_WL2_CORE_0
    run_test(isMax);

    printf("WL3 Single Core. \nMax CPU Freq: %d.\n", isMax);
    REGISTER_WL3_CORE_0
    run_test(isMax);

}

void multi_core_tests(int isMax){
                       // Initialize the workload(s)
    //Two different workloads on cores 0 and 1, respectively, simultaneously
    printf("WL1 Core 0\nWL2 Core 1\n. Max CPU Freq: %d.\n", isMax);
    REGISTER_WL1_CORE_0
    REGISTER_WL2_CORE_1
    run_test(isMax);

    printf("WL1 Core 0\nWL3 Core 1\n. Max CPU Freq: %d.\n", isMax);
    REGISTER_WL1_CORE_0
    REGISTER_WL3_CORE_1
    run_test(isMax);

    printf("WL2 Core 0\nWL3 Core 1\n. Max CPU Freq: %d.\n", isMax);
    REGISTER_WL2_CORE_0
    REGISTER_WL3_CORE_1
    run_test(isMax);

    //Same workloads on cores 0 and 1 simultaneously
    printf("WL1 Core 0 & Core 1\n. Max CPU Freq: %d.\n", isMax);
    REGISTER_WL1_CORE_0
    REGISTER_WL1_CORE_1
    run_test(isMax);

    printf("WL2 Core 0 & Core 1\n. Max CPU Freq: %d.\n", isMax);
    REGISTER_WL2_CORE_0
    REGISTER_WL2_CORE_1
    run_test(isMax);

    printf("WL3 Core 0 & Core 1\n. Max CPU Freq: %d.\n", isMax);
    REGISTER_WL3_CORE_0
    REGISTER_WL3_CORE_1
    run_test(isMax);
}


void run_tests(){
    int isMax;
    for (isMax=FALSE; isMax <= TRUE; isMax++){
        printf("Initialization: Single Core Tests.\n");
        single_core_tests(isMax);
        printf("Initialization: Multi-core Tests.\n");
        multi_core_tests(isMax);
    }
}


int main(int argc, char *argv[]) {
    run_tests();
    return 0;
}
