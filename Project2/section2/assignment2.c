// UCSD CSE237A - WI18
// Important! You WILL NEED TO IMPLEMENT AND SUBMIT this file.
// For more details, please see the instructions in the class website.
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
#include "shared_var.h"

//typedef struct
//{    int wl;
//    int time;
//} WLxTime;

const static char* max = "MAX";
const static char* min = "MIN";
const static char* exec = "Execution Time";
const static char* crit = "Critical Time";
// Functions related to Sample code 3.
static void report_measurement(int, PerfData*);
static void* sample3_init(void*);
static void* sample3_body(void*);
static void* sample3_exit(void*);

static void* workloads_init(void*);
static void* workloads_body(void*);
static void* workloads_exit(void*);

static void test_schedule(SharedVariable* v){
  int running_time_in_sec = 10;

  program_init(v);
  // Initialize the sheduler
  init_scheduler(v);
  unregister_workload_all(); // Unregister all workload in profiling (if any)
  set_userspace_governor(); // Change the governor (in case if needed)

  // Do scheduling
  printf("Start scheduling.\n");
  TimeType start_sched_time = get_current_time_us();
  while ((v->bProgramExit != 1) &&
         ((get_current_time_us() - start_sched_time) <
          running_time_in_sec * 1000 * 1000)) {
    start_scheduling(v);
    schedule();
    finish_scheduling(v);
  }

  // exit scheduling
  exit_scheduler();
  set_ondemand_governor();
  program_exit(v);
}
static const char* set_freq_get_string(int isMax) {

  const char *freq;
  if (isMax) {
    set_by_max_freq();
    freq = max;
  } else {
    set_by_min_freq();
    freq = min;
  }
  return freq;
}

static const char* get_sorting_criteria_string(int is_exec) {
  const char* sorting_criteria;
  if (is_exec){
    sorting_criteria = exec;
  } else{
    sorting_criteria = crit;
  }
  return sorting_criteria;
}

static void run_workloads_sequential(int isMax, SharedVariable* sv)  {
 const char* freq = set_freq_get_string(isMax);

  int num_workloads = get_num_workloads();
  int w_idx;
  int num_iterations = 10;
  printf("at %s freq.\n", freq);

  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    const WorkloadItem *workload_item = get_workload(w_idx);

    void *init_ret = workload_item->workload_init(NULL);

//    printf("Workload body %2d starts.\n", w_idx);
    long long curTime = get_current_time_us();
    void *body_ret = workload_item->workload_body(init_ret);
//    long long total_time = get_current_time_us() - curTime;
    int i_time = (int) (get_current_time_us() - curTime);
//    printf("Workload body %2d finishes in %lld <=> %d \xC2\xB5s.\n", w_idx, total_time, i_time);
//    printf("Workload body %2d finishes in %d \xC2\xB5s.\n", w_idx, i_time);

    sv->workloads[w_idx].wl = w_idx;
    sv->workloads[w_idx].time = i_time;

    void *exit_ret = workload_item->workload_exit(init_ret);
  }
  //average execution times
  for (int iterations = 1; iterations <num_iterations; ++iterations) {
    for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
      const WorkloadItem *workload_item = get_workload(w_idx);

      void *init_ret = workload_item->workload_init(NULL);

//      printf("Workload body %2d starts.\n", w_idx);
      long long curTime = get_current_time_us();
      void *body_ret = workload_item->workload_body(init_ret);
//    long long total_time = get_current_time_us() - curTime;
      int i_time = (int) (get_current_time_us() - curTime);
//    printf("Workload body %2d finishes in %lld <=> %d \xC2\xB5s.\n", w_idx, total_time, i_time);
//      printf("Workload body %2d finishes in %d \xC2\xB5s.\n", w_idx, i_time);

//      sv->workloads[w_idx].wl = w_idx;
      sv->workloads[w_idx].time += i_time ;
      if (iterations + 1 == num_iterations){
        sv->workloads[w_idx].time /= num_iterations ;
        printf("Workload body %2d finishes in %d \xC2\xB5s (AVG).\n", w_idx, sv->workloads[w_idx].time);
      }

      void *exit_ret = workload_item->workload_exit(init_ret);
    }
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

//hope int is big enough to receive value
static int inline calculate_critical_value(int* crit_val_table, bool is_successor[NUM_WORKLOADS][NUM_WORKLOADS], int
workload_index,
                                           SharedVariable * sv ){
  if (crit_val_table[workload_index] == -1){ //calculate the critval

    //find max crit val of all is_successor
    //NOTE: for this program, the dependency graph is many-to-one, i.e., each state only has one successor;
    // so the inner if statement only executes once
    int max_val = 0;
    for (int other_workload = 0; other_workload < NUM_WORKLOADS; ++other_workload) {
      if (is_successor[workload_index][other_workload] ){
        //valid successor
        int successor_crit_time = calculate_critical_value(crit_val_table, is_successor, other_workload, sv);
        if (successor_crit_time > max_val){
          max_val = successor_crit_time;
        }
      }
    }
    crit_val_table[workload_index] = sv->workloads[workload_index].time + max_val;
  }
  return crit_val_table[workload_index];
}

static inline void get_critical_path(SharedVariable* sv) {
  int num_workloads = get_num_workloads();
  int w_idx;
  bool* is_starting_tasks = (bool*)malloc(num_workloads * sizeof(bool));
  bool* is_ending_tasks = (bool*)malloc(num_workloads * sizeof(bool));

//init table
  int c_path_DP_table[NUM_WORKLOADS];
  memset(c_path_DP_table, -1, sizeof(c_path_DP_table));



  // 1. Find all starting tasks
  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    is_starting_tasks[w_idx] = true;
  }
  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    int successor_idx = get_workload(w_idx)->successor_idx;
    if (successor_idx == NULL_TASK){
      is_ending_tasks[w_idx] = true;
      continue;
    }
    is_ending_tasks[w_idx] = false;
    is_starting_tasks[successor_idx] = false;
  }

  //set the c_path_table values for states without is_successor.
  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    if(is_ending_tasks[w_idx]) {
      c_path_DP_table[w_idx] = sv->workloads[w_idx].time;
    }
  }

  // 2. set successor adjacency list
  bool is_successor[NUM_WORKLOADS][NUM_WORKLOADS];
  memset(is_successor, 0, sizeof(is_successor[0][0]) * NUM_WORKLOADS * NUM_WORKLOADS);

//set successors
  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    if (!is_starting_tasks[w_idx])
      continue;

    int orig_state = w_idx;
    int successor_state = get_workload(w_idx)->successor_idx;
    while (successor_state != NULL_TASK) {
      is_successor[orig_state][successor_state] = true;
      orig_state = successor_state;
      successor_state = get_workload(orig_state)->successor_idx;
    }
  }



  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    int crit_time = calculate_critical_value(c_path_DP_table, is_successor, w_idx, sv);

    //since each state only has one successor; accumulate time from start to finish
//    int crit_time_alt = sv->workloads[w_idx].time;
//    int successor_idx = get_workload(w_idx)->successor_idx;
////    printf("%2d", w_idx);
//
//    while (successor_idx != NULL_TASK) {
//
//      crit_time_alt += sv->workloads[successor_idx].time;
////      printf("( -> %2d", successor_idx);
//
//      successor_idx = get_workload(successor_idx)->successor_idx;
//    }
//
//    printf("DP: %d\n Sum: %d\n\n", crit_time, crit_time_alt);
//    assert(crit_time == crit_time_alt);

    //replace time with this after asserts look good;
    // saves space since we don't need process-specific
    // speed?
    sv->workloads[w_idx].crit_time = crit_time;
  }


  free(is_starting_tasks);
  free(is_ending_tasks);
}

static void profile_sample_workloads(){
  register_workload(0, sample3_init, sample3_body, sample3_exit);
  PerfData perf_msmts[MAX_CPU_IN_RPI3];
  run_workloads(perf_msmts);
  report_measurement(get_cur_freq(), perf_msmts);
  unregister_workload_all();
}

static void profile_real_workloads(){
  register_workload(0, workloads_init, workloads_body, workloads_exit);
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
  sv->is_first_run = true;
	// TODO: Fill the body

	// This function is executed before the scheduling simulation.


	// You need to characterize the workloads (e.g., the execution time and
    // memory access patterns) with the task graphs

	// Tip 1. You can get the current time here like:
	// long long curTime = get_current_time_us();

    // Tip 2. You can also use your kernel module to read PMU events,
    // and provided workload profiling code in the same way to the part 1.



  //do via exec

  for (int is_max_freq = 0; is_max_freq <= 1; ++is_max_freq){
    sv->is_max_freq = (bool)is_max_freq;
    //////////////////////////////////////////////////////////////
    // Sample code 1: Running all tasks on the current running core
    // This executes all tasks one-by-one at the maximum frequency
    //////////////////////////////////////////////////////////////

    run_workloads_sequential(is_max_freq, sv);
    //////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////
    // Sample code 2: Print a task path for each starting task
    // This prints the task path from each statring task to the end
    //////////////////////////////////////////////////////////////
    print_task_path();
    get_critical_path(sv);

    qsort(sv->workloads, (size_t) NUM_WORKLOADS, sizeof(WLxTime), compare_exec_time);
    sv->is_exec_time = 1;
    test_schedule(sv);

    sv->is_first_run = false;

    qsort(sv->workloads, (size_t) NUM_WORKLOADS, sizeof(WLxTime), compare_crit_time);
    sv->is_exec_time = 0;
    test_schedule(sv);

  }

printf("\t\t\t--Optimal Schedule--\n");
  const char *freq = set_freq_get_string(sv->is_max_freq_best);
  const char* sorting_criteria = get_sorting_criteria_string(sv->is_exec_time_best);

  printf("Sorted by: %s\n", sorting_criteria);
  printf("Freq:  %s\n", freq);
  printf("Priority List:\n");
  sv->is_max_freq =   sv->is_max_freq_best;
  for (int w_idx = 0; w_idx < NUM_WORKLOADS; ++w_idx) {
    sv->workloads[w_idx] = sv->workloads_best_ordering[w_idx];
//    printf("old w_idx %d ", sv->workloads[w_idx].wl);
//
//    sv->workloads[w_idx].wl = sv->workloads_best_ordering[w_idx].wl;
//    sv->workloads[w_idx].time = sv->workloads_best_ordering[w_idx].time;
    printf("\t%d: WL %d\n", w_idx, sv->workloads[w_idx].wl);
  }



  //////////////////////////////////////////////////////////////
    
    //////////////////////////////////////////////////////////////
    // Sample code 3: Profile a sequence of workloads with PMU
    // This profiles the body functions of the first five tasks.
    // See also sample3_init(), sample3_body(), and sample3_exit()
    // at the end of this file.
    //////////////////////////////////////////////////////////////

//  profile_sample_workloads();
  profile_real_workloads();
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

static inline TaskSelection LJF_scheduler(SharedVariable *sv, const int core,
                                          const int num_workloads,
                                          const bool *schedulable_workloads, const bool *finished_workloads){
  //////////////////////////////////////////////////////////////
  // Sample scheduler: A naive scheduler that satisfies the task dependency
  // This scheduler selects a possible task in order of the task index,
  // and always uses the minumum frequency.
  // This doesn't guarantee any of task deadlines.
  //////////////////////////////////////////////////////////////
  TaskSelection task_selection;

  // Choose frequency
  if (sv->is_max_freq){
    task_selection.freq = FREQ_CTL_MAX; // You can change this to FREQ_CTL_MAX
  } else{
    task_selection.freq = FREQ_CTL_MIN;
  }


  //set freq dynamically if next shortest job is 2x the time of this job? assumes CPU bound

  int w_idx;
  int prospective_workload;


  if (true){//core == 0
    //get a long job
    for (w_idx = num_workloads-1; w_idx >= 0; --w_idx) {
      // Choose one possible task
      prospective_workload = sv->workloads[w_idx].wl;
//      printf("best wl_idx = %2d\n", prospective_workload);
      if (finished_workloads[prospective_workload] || sv->scheduledWorkloads[prospective_workload]){
        continue;
      }
      if (schedulable_workloads[prospective_workload]) {//iterate over sorted workloads
        // available
        task_selection.task_idx = prospective_workload;
        sv->scheduledWorkloads[prospective_workload] = 1;
        break;
      }
    }
  }

  else {//get a short job

    for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
      // Choose one possible task
      prospective_workload = sv->workloads[w_idx].wl;
      if (finished_workloads[prospective_workload] || sv->scheduledWorkloads[prospective_workload]){
        continue;
      }
      else if (schedulable_workloads[prospective_workload] ) {//iterate over sorted workloads
        // and do the next one
        // available
        task_selection.task_idx = prospective_workload;
        sv->scheduledWorkloads[prospective_workload] = 1;
        break;
      }
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



    return LJF_scheduler(sv, core, num_workloads, schedulable_workloads, finished_workloads);
    //////////////////////////////////////////////////////////////
}

// This function is called before scheduling 16 tasks.
// You may implement some code to evaluate performance and power consumption.
// (This is called in main_section2.c)

//custom fast sort since we know the # of workloads in advance.
static inline void sorting_network(WLxTime * workloads){
  #define min(x, y) (((x).time)<((y).time)?(x):(y))
  #define max(x, y) (((x).time)<((y).time)?(y):(x))
  #define SWAP(x,y) { const WLxTime shorter = min(workloads[x], workloads[y]); \
                    const WLxTime longer = max(workloads[x], workloads[y]); \
                    workloads[x] = shorter; workloads[y] = longer; }
  SWAP(0, 1);
  SWAP(2, 3);
  SWAP(4, 5);
  SWAP(6, 7);
  SWAP(8, 9);
  SWAP(10, 11);
  SWAP(12, 13);
  SWAP(14, 15);
  SWAP(0, 2);
  SWAP(4, 6);
  SWAP(8, 10);
  SWAP(12, 14);
  SWAP(1, 3);
  SWAP(5, 7);
  SWAP(9, 11);
  SWAP(13, 15);
  SWAP(0, 4);
  SWAP(8, 12);
  SWAP(1, 5);
  SWAP(9, 13);
  SWAP(2, 6);
  SWAP(10, 14);
  SWAP(3, 7);
  SWAP(11, 15);
  SWAP(0, 8);
  SWAP(1, 9);
  SWAP(2, 10);
  SWAP(3, 11);
  SWAP(4, 12);
  SWAP(5, 13);
  SWAP(6, 14);
  SWAP(7, 15);
  SWAP(5, 10);
  SWAP(6, 9);
  SWAP(3, 12);
  SWAP(13, 14);
  SWAP(7, 11);
  SWAP(1, 2);
  SWAP(4, 8);
  SWAP(1, 4);
  SWAP(7, 13);
  SWAP(2, 8);
  SWAP(11, 14);
  SWAP(2, 4);
  SWAP(5, 6);
  SWAP(9, 10);
  SWAP(11, 13);
  SWAP(3, 8);
  SWAP(7, 12);
  SWAP(6, 8);
  SWAP(10, 12);
  SWAP(3, 5);
  SWAP(7, 9);
  SWAP(3, 4);
  SWAP(5, 6);
  SWAP(7, 8);
  SWAP(9, 10);
  SWAP(11, 12);
  SWAP(6, 7);
  SWAP(8, 9);
#undef SWAP
#undef min
#undef max
}
void start_scheduling(SharedVariable* sv) {
	// TODO: Fill the body if needed
  sv->start_time = get_current_time_us();
//  long long s_start_time = get_current_time_us();
//  sorting_network(sv->workloads);
//  int sn_time = (int)(get_current_time_us()-s_start_time);
//  printf("Sorting Networks takes %d \xC2\xB5s.\n",sn_time);
//
//  WLxTime copy[16];
//  memcpy(copy, sv->workloads, sizeof(sv->workloads));
//
//  long long q_start_time = get_current_time_us();
//  qsort( copy, (size_t)NUM_WORKLOADS, sizeof(WLxTime), compare_exec_time );
//  int qs_time = (int)(get_current_time_us()-q_start_time);
//  printf("Quicksort takes %d \xC2\xB5s.\n", qs_time);
//
//    for (int w_idx = 0; w_idx < NUM_WORKLOADS; ++w_idx) {
//    assert(copy[w_idx].wl == sv->workloads[w_idx].wl);
////    printf("Workload %2d takes %d \xC2\xB5s.\n", arr[w_idx].wl, arr[w_idx].time);
//  }
//  qsort(sv->workloads, (size_t) NUM_WORKLOADS, sizeof(WLxTime), compare_exec_time);
  memset(sv->scheduledWorkloads, 0, sizeof(sv->scheduledWorkloads));
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
  int curr_freq_power;
  if (sv->is_max_freq){
    curr_freq_power = 1050;
  } else{
    curr_freq_power = 450;
  }
  long long time = (get_current_time_us() - sv->start_time);
  int sec = 1000 * 1000;
  double pow =  (((double)(time)/(double)(sec))
                         * curr_freq_power);//if max
  printf("Power: %f mW.\nRun Time: %lld\xC2\xB5s.\n", pow, time);

  if(time < sec && (sv->is_first_run ||  pow < sv->best_pow)){
    sv->is_first_run = false;
    for (int w_idx = 0; w_idx < NUM_WORKLOADS; ++w_idx) {//maybe this'll do it
      printf("best wl_idx = %2d\n", sv->workloads[w_idx].wl);
//
//      sv->workloads_best_ordering[w_idx].wl = sv->workloads[w_idx].wl;
//      sv->workloads_best_ordering[w_idx].time = sv->workloads[w_idx].time;
//
    }
    memcpy(&sv->workloads_best_ordering, &sv->workloads, sizeof(sv->workloads));
    sv->is_max_freq_best = sv->is_max_freq;
    sv->best_pow = pow;
    sv->is_exec_time_best = sv->is_exec_time;
  }// else don't pick anything that exceeds our deadline
    //TODO: find some way to average the runs out in case it picks an outlier
    //i.e. if is_max_freq == is_max_freq_best && is_exec_time_sorted == is_exec_time_sorted_best .. etc.
//  else if (time < 1000*1000 && pow < sv->best_pow){ //if under threshold, choose by best power
//    memcpy(sv->workloads_best_ordering, sv->workloads, sizeof(sv->workloads_best_ordering));
//    sv->is_max_freq_best = sv->is_max_freq;
//
//  }

// of this is our last time out of here, set this so the real scheduling
// (i.e. next iteration) will know which frequency to use.
  //sv->is_max_freq =   sv->is_max_freq_best;

  //if averaging, put counter in sv that increments each time we have the same configuration (since it happens ~6
  // times in a row), zero out before we schedule.

  //TODO: put is_final_run field? to fine grain checks?


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
        TimeType time_estimated = (TimeType)pf->cc/(TimeType)((double)freq/1000);
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

static void* real_ret_workload[NUM_WORKLOADS];
static void* workloads_init(void* unused) {
  for (int i=0; i < NUM_WORKLOADS; ++i){
    real_ret_workload[i] = get_workload(i)->workload_init(NULL);
  }
}
static void* workloads_body(void* unused) {
  for (int i=0; i < NUM_WORKLOADS; ++i){
    get_workload(i)->workload_body(real_ret_workload[i]);
  }
}
static void* workloads_exit(void* unused) {
  for (int i=0; i < NUM_WORKLOADS; ++i){
    get_workload(i)->workload_exit(real_ret_workload[i]);
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
