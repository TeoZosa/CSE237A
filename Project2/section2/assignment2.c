// UCSD CSE237A - WI18
// Important! You WILL NEED TO IMPLEMENT AND SUBMIT this file.
// For more details, please see the instructions in the class website.
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "section1.h"
#include "section2.h"

// You can access the workloads with the task graph
// using the following header file
#include "workload.h"

// You can characterize the workload using the provided funtions of
// these header files in the same way to the part 1
#include "workload_util.h"
#include "cpufreq.h"
#include "pmu_reader.h"

// FREQ_CTL_MIN, FREQ_CTL_MAX is defined here
#include "scheduler.h"      

//typedef struct
//{    int wl;
//    int time;
//} WLxTime;

const static char* max = "MAX";
const static char* min = "MIN";
// Functions related to Sample code 3.
static void report_measurement(int, PerfData*);
static void* sample3_init(void*);
static void* sample3_body(void*);
static void* sample3_exit(void*);


static void run_workloads_sequential(int isMax, SharedVariable* sv)  {
WLxTime arr[16];
 const char* freq;
  if (isMax){
    set_by_max_freq();
    freq = max;
  }
  else{
    set_by_min_freq();
    freq = min;
  }

  int num_workloads = get_num_workloads();
  int w_idx;
  printf("at %s freq.\n", freq);
  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    const WorkloadItem* workload_item = get_workload(w_idx);

    void* init_ret = workload_item->workload_init(NULL);

    printf("Workload body %2d starts.\n", w_idx);
    long long curTime = get_current_time_us();
    void* body_ret = workload_item->workload_body(init_ret);
//    long long total_time = get_current_time_us() - curTime;
    int i_time = (int) (get_current_time_us() - curTime);
//    printf("Workload body %2d finishes in %lld <=> %d \xC2\xB5s.\n", w_idx, total_time, i_time);
    printf("Workload body %2d finishes in %d \xC2\xB5s.\n", w_idx, i_time);

    sv->workloads[w_idx].wl = w_idx;
    sv->workloads[w_idx].time = i_time;

    void* exit_ret = workload_item->workload_exit(init_ret);
  }



  }



static void print_task_path() {
  int num_workloads = get_num_workloads();
  int w_idx;
  bool* is_starting_tasks = (bool*)malloc(num_workloads * sizeof(bool));

  // 1. Find all starting tasks
  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    is_starting_tasks[w_idx] = true;
  }
  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    int successor_idx = get_workload(w_idx)->successor_idx;
    if (successor_idx == NULL_TASK)
      continue;
    is_starting_tasks[successor_idx] = false;
  }

  // 2. Print the path for each starting task
  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    if (!is_starting_tasks[w_idx])
      continue;

    printf("%2d", w_idx);
    int successor_idx = get_workload(w_idx)->successor_idx;
    while (successor_idx != NULL_TASK) {
      printf(" -> %2d", successor_idx);
      successor_idx = get_workload(successor_idx)->successor_idx;
    }
    printf("\n");
  }

  free(is_starting_tasks);
}

static void profile_workloads(){
  register_workload(0, sample3_init, sample3_body, sample3_exit);
  PerfData perf_msmts[MAX_CPU_IN_RPI3];
  run_workloads(perf_msmts);
  report_measurement(get_cur_freq(), perf_msmts);
  unregister_workload_all();
}


// You can characterize the given workloads with the task graph
// in this function to make your scheduling strategies.
// This is called the start part of the program before the scheduling.
// You need to learn the characteristics within 300 seconds.
// (See main_section2.c)

void learn_workloads(SharedVariable* sv) {

	// TODO: Fill the body

	// This function is executed before the scheduling simulation.


	// You need to characterize the workloads (e.g., the execution time and
    // memory access patterns) with the task graphs

	// Tip 1. You can get the current time here like:
	// long long curTime = get_current_time_us();

    // Tip 2. You can also use your kernel module to read PMU events,
    // and provided workload profiling code in the same way to the part 1.

    //////////////////////////////////////////////////////////////
    // Sample code 1: Running all tasks on the current running core
    // This executes all tasks one-by-one at the maximum frequency
    //////////////////////////////////////////////////////////////

//  run_workloads_sequential(0);//MIN
  run_workloads_sequential(1, sv);//MAX
    //////////////////////////////////////////////////////////////
    
    //////////////////////////////////////////////////////////////
    // Sample code 2: Print a task path for each starting task
    // This prints the task path from each statring task to the end
    //////////////////////////////////////////////////////////////
  print_task_path();

    //////////////////////////////////////////////////////////////
    
    //////////////////////////////////////////////////////////////
    // Sample code 3: Profile a sequence of workloads with PMU
    // This profiles the body functions of the first five tasks.
    // See also sample3_init(), sample3_body(), and sample3_exit()
    // at the end of this file.
    //////////////////////////////////////////////////////////////

 profile_workloads();
    //////////////////////////////////////////////////////////////
}



static inline TaskSelection naive_scheduler(SharedVariable* sv, const int core,
                                            const int num_workloads,
                                            const bool* schedulable_workloads, const bool* finished_workloads){
  //////////////////////////////////////////////////////////////
  // Sample scheduler: A naive scheduler that satisfies the task dependency
  // This scheduler selects a possible task in order of the task index,
  // and always uses the minumum frequency.
  // This doesn't guarantee any of task deadlines.
  //////////////////////////////////////////////////////////////
  TaskSelection task_selection;

  // Choose the minimum frequency
  task_selection.freq = FREQ_CTL_MIN; // You can change this to FREQ_CTL_MAX

  int w_idx;
  int selected_worload_idx;

  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    // Choose one possible task
    if (schedulable_workloads[w_idx]) {
      task_selection.task_idx = w_idx;
      break;
    }
  }
  return task_selection;
}

static inline TaskSelection EDF_scheduler(SharedVariable* sv, const int core,
                                            const int num_workloads,
                                            const bool* schedulable_workloads, const bool* finished_workloads){
  //////////////////////////////////////////////////////////////
  // Sample scheduler: A naive scheduler that satisfies the task dependency
  // This scheduler selects a possible task in order of the task index,
  // and always uses the minumum frequency.
  // This doesn't guarantee any of task deadlines.
  //////////////////////////////////////////////////////////////
  TaskSelection task_selection;

  // Choose the minimum frequency
  task_selection.freq = FREQ_CTL_MAX; // You can change this to FREQ_CTL_MAX


  //set freq dynamically if next shortest job is 2x the time of this job? assumes CPU bound

  int w_idx;
  int next_shortest_workload;

  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    // Choose one possible task
    next_shortest_workload = sv->workloads[w_idx].wl;
    if (schedulable_workloads[next_shortest_workload]) {//iterate over sorted workloads and do the next one available
      task_selection.task_idx = next_shortest_workload;
      break;
    }
  }
  return task_selection;
}
// Select a workload index among schedulable workloads.

// This is called by the provided scheduler base (schedule() function.)

              /* You have to return a TaskSelection structure specifying
               * the selected task index and the frequency (FREQ_CTL_MIN or FREQ_CTL_MAX)
               * that you want to execute.
                 */
TaskSelection select_workload(
        SharedVariable* sv, const int core,
        const int num_workloads,
        const bool* schedulable_workloads, const bool* finished_workloads) {

                // TODO: Fill the body

                // This function is executed inside of the provided scheduler code.
    // You need to implement an energy-efficient LIST scheduler.



    return EDF_scheduler(sv, core, num_workloads, schedulable_workloads, finished_workloads);
    //////////////////////////////////////////////////////////////
}

// This function is called before scheduling 16 tasks.
// You may implement some code to evaluate performance and power consumption.
// (This is called in main_section2.c)

static inline void sort6_sorting_network_simple_swap(int * d){
#define min(x, y) (x<y?x:y)
#define max(x, y) (x<y?y:x)
#define SWAP(x,y) { const int a = min(d[x], d[y]); \
                    const int b = max(d[x], d[y]); \
                    d[x] = a; d[y] = b; }
  SWAP(1, 2);
  SWAP(4, 5);
  SWAP(0, 2);
  SWAP(3, 5);
  SWAP(0, 1);
  SWAP(3, 4);
  SWAP(1, 4);
  SWAP(0, 3);
  SWAP(2, 5);
  SWAP(1, 3);
  SWAP(2, 4);
  SWAP(2, 3);
#undef SWAP
#undef min
#undef max
}
void start_scheduling(SharedVariable* sv) {
	// TODO: Fill the body if needed
  sv->start_time = get_current_time_us();
  qsort( sv->workloads, (size_t)NUM_WORKLOADS, sizeof(WLxTime), compare );

//  printf("Workloads sorted:\n");
//  for (int w_idx = 0; w_idx < NUM_WORKLOADS; ++w_idx) {
//    sv->sortedWorkloads[w_idx] = (unsigned short int) sv->workloads[w_idx].wl;
////    printf("Workload %2d takes %d \xC2\xB5s.\n", arr[w_idx].wl, arr[w_idx].time);
//  }
}

// This function is called whenever all workloads in the task graph 
// are finished. You may evaluate performance and power consumption 
// of your implementation here.
// (This is called in main_section2.c)
void finish_scheduling(SharedVariable* sv) {
	// TODO: Fill the body if needed
  int time = (int)(get_current_time_us() - sv->start_time);
  double pow = (double) ((
                                 (time)/(1000 * 1000))
                         * (1050));//if max
  printf("Power: %f mW.\nRun Time: %d\xC2\xB5s.\n", pow, time);

}

/////////////////////////////////////////////////////////////////////////////
// From here, We provide functions related to Sample 3.
/////////////////////////////////////////////////////////////////////////////

// This function is same to one provided in part 1.
static void report_measurement(int freq, PerfData* perf_msmts) {
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

// Initialize the first five tasks
static void* p_ret_workload[5];
static void* sample3_init(void* unused) {
    p_ret_workload[0] = get_workload(0)->workload_init(NULL);
    p_ret_workload[1] = get_workload(1)->workload_init(NULL);
    p_ret_workload[2] = get_workload(2)->workload_init(NULL);
    p_ret_workload[3] = get_workload(3)->workload_init(NULL);
    p_ret_workload[4] = get_workload(4)->workload_init(NULL);
}

// Execute the body of the first three tasks sequentially
static void* sample3_body(void* unused) {
    get_workload(0)->workload_body(p_ret_workload[0]);
    get_workload(1)->workload_body(p_ret_workload[1]);
    get_workload(2)->workload_body(p_ret_workload[2]);
    get_workload(3)->workload_body(p_ret_workload[3]);
    get_workload(4)->workload_body(p_ret_workload[4]);
}

// Execute the exit function of the first five tasks sequentially
static void* sample3_exit(void* unused) {
    get_workload(0)->workload_exit(p_ret_workload[0]);
    get_workload(1)->workload_exit(p_ret_workload[1]);
    get_workload(2)->workload_exit(p_ret_workload[2]);
    get_workload(3)->workload_exit(p_ret_workload[3]);
    get_workload(4)->workload_exit(p_ret_workload[4]);
}
