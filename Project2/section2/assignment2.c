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

const static char* max = "MAX";
const static char* min = "MIN";
const static char* exec = "Execution Time";
const static char* crit = "Critical Time";
// Functions related to Sample code 3.
static void report_measurement(int, PerfData*);
static void* sample3_init(void*);
static void* sample3_body(void*);
static void* sample3_exit(void*);

//inits the real workloads
static void* workloads_init(void*);
static void* workloads_body(void*);
static void* workloads_exit(void*);

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

static double cumulative_moving_avg(double avg, double new_datum, unsigned short int num_data){
  //num data points NOT including curr data point
  return (new_datum + (num_data) * avg)/(num_data+1);
}

static void update_SV_avg(SharedVariable* sv, double new_pow_datum, long long new_time_datum) {
  unsigned short int num_data = sv->times_run_curr_schedule;
  double pow_avg = sv->avg_pow_curr_schedule; //note: if this is garbage, should be multiplied by 0 anyway
  sv->avg_pow_curr_schedule = cumulative_moving_avg(pow_avg, new_pow_datum, num_data);
  double time_avg = sv->avg_time_curr_schedule; //note: if this is garbage, should be multiplied by 0 anyway
  sv->avg_time_curr_schedule = cumulative_moving_avg(time_avg, new_time_datum, num_data);
  ++sv->times_run_curr_schedule;//includes curr_data point
  printf("Curr Power: %f\n", new_pow_datum);
  printf("Average Power: %f\n", sv->avg_pow_curr_schedule);

}

static void init_for_scheduling(SharedVariable* sv){
  sv->schedule_feasible = true;
  sv->times_run_curr_schedule = 0;
  sv->avg_pow_curr_schedule = 0;
  sv->avg_time_curr_schedule = 0;
}


static void check_if_tested_schedule_is_better(SharedVariable* sv){
  const bool under_time_threshold = sv->schedule_feasible;
  const double curr_schedule_power = sv->avg_pow_curr_schedule;
  const double curr_schedule_time = sv->avg_time_curr_schedule;

  if (under_time_threshold && (sv->no_best_schedule_yet || curr_schedule_power < sv->best_pow) ){
    sv->no_best_schedule_yet = false; //redundant if I set best pow's initial value very high?
    memcpy(&sv->workloads_best_ordering, &sv->workloads, sizeof(sv->workloads));
    sv->is_max_freq_best = sv->is_max_freq;
    sv->best_pow = curr_schedule_power;
    sv->best_time = curr_schedule_time;
    sv->is_exec_time_best = sv->is_exec_time;
  }
}

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


static void run_test_schedule_single(int sort_by_exec_time, SharedVariable* sv) {
  init_for_scheduling(sv);
  sv->is_exec_time = (bool)sort_by_exec_time;
  if (sort_by_exec_time){
    qsort(sv->workloads, (size_t) NUM_WORKLOADS, sizeof(WLxTime), compare_exec_time);
  } else{
    qsort(sv->workloads, (size_t) NUM_WORKLOADS, sizeof(WLxTime), compare_crit_time);
  }
  test_schedule(sv);
  check_if_tested_schedule_is_better(sv);
}

static void run_test_schedule_all(SharedVariable* sv) {
  for (int sort_by_exec_time = 0; sort_by_exec_time <= 0; ++sort_by_exec_time){
    printf("Sorted By: %s\n", get_sorting_criteria_string(sort_by_exec_time));
    run_test_schedule_single(sort_by_exec_time, sv);
  }
}

static inline void set_best_schedule_and_print(SharedVariable* sv) {
//  printf("\t\t\t--Optimal Schedule--\n");
//  const char *freq = set_freq_get_string(sv->is_max_freq_best);
//  const char* sorting_criteria = get_sorting_criteria_string(sv->is_exec_time_best);

//  printf("Freq:  %s\n", freq);
//  printf("Sorted by: %s\n", sorting_criteria);
//  printf("Average Power: %f\n", sv->best_pow);
////  printf("Average Time: %lld\xC2\xB5s.\n", sv->best_pow);
  printf("Priority List:\n\n");
  sv->is_max_freq =   sv->is_max_freq_best;

  for (int w_idx = 0; w_idx < NUM_WORKLOADS; ++w_idx) {
    sv->workloads[w_idx] = sv->workloads_best_ordering[w_idx];
    const char *curr_freq = set_freq_get_string(sv->workloads[w_idx].maxFreq);
    printf("%d: \tWL %d\tFreq %d\n", w_idx, sv->workloads[w_idx].wl, sv->workloads[w_idx].maxFreq);
  }
}

//////////////////////////////////////////////////////////////
// Sample code 1: Running all tasks on the current running core
// This executes all tasks one-by-one at the maximum frequency
//////////////////////////////////////////////////////////////


static void run_workloads_sequential(int isMax, SharedVariable* sv)  {
 const char* freq = set_freq_get_string(isMax);

  int num_workloads = get_num_workloads();
  int w_idx;
  int num_iterations = 10;
//  printf("\tAt Freq = %s\n", freq);

  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    const WorkloadItem *workload_item = get_workload(w_idx);

    void *init_ret = workload_item->workload_init(NULL);

    long long curTime = get_current_time_us();
    void *body_ret = workload_item->workload_body(init_ret);
    int i_time = (int) (get_current_time_us() - curTime);
    sv->workloads[w_idx].wl = w_idx;
    sv->workloads[w_idx].time = i_time;
    sv->workloads[w_idx].maxFreq = isMax;
    void *exit_ret = workload_item->workload_exit(init_ret);
  }
  //average execution times
  for (int iterations = 1; iterations <num_iterations; ++iterations) {
    //note: iterating over all workloads in order to avoid spatial locality
    //speeding up the avg runtimes
    for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
      const WorkloadItem *workload_item = get_workload(w_idx);
      void *init_ret = workload_item->workload_init(NULL);
      long long curTime = get_current_time_us();
      void *body_ret = workload_item->workload_body(init_ret);
      int i_time = (int) (get_current_time_us() - curTime);
      sv->workloads[w_idx].time += i_time ;

      if (iterations + 1 == num_iterations){
        sv->workloads[w_idx].time /= num_iterations ;
//        printf("Workload body %2d finishes in %d \xC2\xB5s (AVG).\n", w_idx, sv->workloads[w_idx].time);
      }

      void *exit_ret = workload_item->workload_exit(init_ret);
    }
  }

  printf("\n");
  }

//////////////////////////////////////////////////////////////
// Sample code 2: Print a task path for each starting task
// This prints the task path from each statring task to the end
//////////////////////////////////////////////////////////////

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

static int inline calculate_critical_value(int* crit_val_table, bool is_successor[NUM_WORKLOADS][NUM_WORKLOADS], int
workload_index,
                                           SharedVariable * sv ){
  if (crit_val_table[workload_index] == -1){ //calculate the critval

    //find max crit val of all is_successor
    //NOTE: for this assignment, the dependency graph is many-to-one, i.e., each state only has one successor;
    // so the inner if statement only executes once
    int max_val = 0;
    for (int other_workload = 0; other_workload < NUM_WORKLOADS; ++other_workload) {
      if (is_successor[workload_index][other_workload] ){
        //workload_index -> other_workload
        int successor_crit_time = calculate_critical_value(crit_val_table, is_successor, other_workload, sv);
        if (successor_crit_time > max_val){
          max_val = successor_crit_time;
        }
      }
    }
    crit_val_table[workload_index] = sv->workloads[workload_index].time + max_val;
  } //else critval already in table

  return crit_val_table[workload_index];
}

static inline void crit_time_assert(SharedVariable* sv, int calculated_crit_time, int workload_num){

  //since each state only has one successor; accumulate time from start to finish
    int crit_time_alt = sv->workloads[workload_num].time;
    int successor_idx = get_workload(workload_num)->successor_idx;
    while (successor_idx != NULL_TASK) {
      crit_time_alt += sv->workloads[successor_idx].time;
      successor_idx = get_workload(successor_idx)->successor_idx;
    }
    printf("DP: %d\n Sum: %d\n\n", calculated_crit_time, crit_time_alt);
    assert(calculated_crit_time == crit_time_alt);

}



static inline void get_critical_path(SharedVariable* sv) {
  int num_workloads = get_num_workloads();
  int w_idx;
  bool* is_starting_tasks = (bool*)malloc(num_workloads * sizeof(bool));
  bool* is_ending_tasks = (bool*)malloc(num_workloads * sizeof(bool));

  //init table
  int c_path_DP_table[NUM_WORKLOADS];
  memset(c_path_DP_table, -1, sizeof(c_path_DP_table));



  // 1. Find all starting tasks and ending tasks
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

  //2. set the c_path_table values for terminal states (i.e. without successors)
  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    if(is_ending_tasks[w_idx]) {
      c_path_DP_table[w_idx] = sv->workloads[w_idx].time;
    }
  }

  // 3. set successor adjacency list
  bool is_successor[NUM_WORKLOADS][NUM_WORKLOADS];
  memset(is_successor, 0, sizeof(is_successor[0][0]) * NUM_WORKLOADS * NUM_WORKLOADS);

  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    if (!is_starting_tasks[w_idx]){
      continue;//don't redundantly calculate values
    }
    int orig_state = w_idx;
    int successor_state = get_workload(orig_state)->successor_idx;
    while (successor_state != NULL_TASK) {
      is_successor[orig_state][successor_state] = true;
      orig_state = successor_state;
      successor_state = get_workload(orig_state)->successor_idx;
    }
  }



  for (w_idx = 0; w_idx < num_workloads; ++w_idx) {
    int crit_time = calculate_critical_value(c_path_DP_table, is_successor, w_idx, sv);
//     crit_time_assert(sv, crit_time, w_idx);
    sv->workloads[w_idx].crit_time = crit_time;
  }


  free(is_starting_tasks);
  free(is_ending_tasks);
}

//////////////////////////////////////////////////////////////
// Sample code 3: Profile a sequence of workloads with PMU
// This profiles the body functions of the first five tasks.
// See also sample3_init(), sample3_body(), and sample3_exit()
// at the end of this file.
//////////////////////////////////////////////////////////////


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
  sv->no_best_schedule_yet = true;
//SharedVariable _svMin;
//  SharedVariable *svMin = &_svMin;
//  program_init(svMin);
//  svMin->no_best_schedule_yet = true;
//	// This function is executed before the scheduling simulation.
//
//
//	// You need to characterize the workloads (e.g., the execution time and
//    // memory access patterns) with the task graphs
//
//
//
//  //do via exec
//  print_task_path();
//  svMin->is_max_freq = (bool)0;
//
//
//  run_workloads_sequential(0, svMin);//updates workload execution times in sv.
//  get_critical_path(svMin);//critical path relies on executions times, which relies on run_workloads
//  run_test_schedule_all(svMin);

  for (int is_max_freq = 1; is_max_freq <= 1; ++is_max_freq){
    sv->is_max_freq = (bool)is_max_freq;


    run_workloads_sequential(is_max_freq, sv);//updates workload execution times in sv.
    get_critical_path(sv);//critical path relies on executions times, which relies on run_workloads
    run_test_schedule_all(sv);
  }

  set_best_schedule_and_print(sv);
//  set_best_schedule_and_print(svMin);

  double time_diff = (1000*1000) - sv->best_time; //should be positive in this case
  double error_term = 50000;//us
  printf("time diff %f\n", time_diff);

    for (int i = NUM_WORKLOADS-1; i >= 0 && time_diff > 0+error_term; i--){
      int wl_time_diff = sv->workloads[i].time;//double time max
      printf("here");
      if (sv->workloads[i].maxFreq && time_diff - wl_time_diff >0){
        printf("%d\n", i);
        sv->workloads[i].maxFreq = 0;
        sv->workloads[i].time *= 2;//replace with old work time
        time_diff -= wl_time_diff;
      }
    }//squeeze it until time diff is negligible

  get_critical_path(sv);
  run_test_schedule_all(sv);
  set_best_schedule_and_print(sv);
  init_scheduler(sv); //reset for the real run

//  profile_sample_workloads();
//  profile_real_workloads();
}


//////////////////////////////////////////////////////////////
// Sample scheduler: A naive scheduler that satisfies the task dependency
// This scheduler selects a possible task in order of the task index,
// and always uses the minumum frequency.
// This doesn't guarantee any of task deadlines.
//////////////////////////////////////////////////////////////
static inline TaskSelection naive_scheduler(SharedVariable* sv, const int core,
                                            const int num_workloads,
                                            const bool* schedulable_workloads, const bool* finished_workloads){

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

/***********
 * Longest Job-First Scheduler: satisfies the task dependency by traversing the priority list from longest to shortest
 * assigning available tasks. In this paradigm, job length implies priority.
 * */

static inline TaskSelection LJF_scheduler(SharedVariable *sv, const int core,
                                          const int num_workloads,
                                          const bool *schedulable_workloads, const bool *finished_workloads){
  TaskSelection task_selection;

  // Choose frequency
//  if (sv->is_max_freq){
//    task_selection.freq = FREQ_CTL_MAX;
//  } else{
//    task_selection.freq = FREQ_CTL_MIN;
//  }

  int w_idx;
  int prospective_workload;
  //iterate over sorted workloads and get the longest available job
  for (w_idx = num_workloads-1; w_idx >= 0; --w_idx) {

    prospective_workload = sv->workloads[w_idx].wl;
    if (finished_workloads[prospective_workload] || sv->scheduledWorkloads[prospective_workload]){
      continue;
    }

    if (schedulable_workloads[prospective_workload]) {
      // available
      task_selection.task_idx = prospective_workload;
      sv->scheduledWorkloads[prospective_workload] = 1;
      if (sv->workloads[w_idx].maxFreq){
        task_selection.freq = FREQ_CTL_MAX;
      } else{
        task_selection.freq = FREQ_CTL_MIN;
      }

      break;
    }

  }

  return task_selection;
}

// This is called by the provided scheduler base (schedule() function.)

/* Returns a TaskSelection structure specifying
 * the selected task index and the frequency (FREQ_CTL_MIN or FREQ_CTL_MAX)
 * that with which to execute.
   */
TaskSelection select_workload(
        SharedVariable* sv, const int core,
        const int num_workloads,
        const bool* schedulable_workloads, const bool* finished_workloads) {
                // This function is executed inside of the provided scheduler code.



    return LJF_scheduler(sv, core, num_workloads, schedulable_workloads, finished_workloads);
    //////////////////////////////////////////////////////////////
}

// This function is called before scheduling 16 tasks.
// You may implement some code to evaluate performance and power consumption.
// (This is called in main_section2.c)
void start_scheduling(SharedVariable* sv) {
  sv->start_time = get_current_time_us();
  memset(sv->scheduledWorkloads, 0, sizeof(sv->scheduledWorkloads));
}

// This function is called whenever all workloads in the task graph 
// are finished. You may evaluate performance and power consumption 
// of your implementation here.
// (This is called in main_section2.c)
void finish_scheduling(SharedVariable* sv) {
  int curr_freq_power;
  if (sv->is_max_freq){
    curr_freq_power = 1050;
  } else{
    curr_freq_power = 450;
  }
  long long time = (get_current_time_us() - sv->start_time);
  int sec = 1000 * 1000;
  double pow =  (((double)(time)/(double)(sec))
                         * curr_freq_power)*2;//two cores
  printf("Power: %f mW.\nRun Time: %lld\xC2\xB5s.\n\n", pow, time);

  long long est_time = 0;
  double est_pow = 0;
  long long this_time;
  int this_freq_power;
  for (int w_idx = 0; w_idx < NUM_WORKLOADS; ++w_idx){
    this_time = sv->workloads[w_idx].time;
    est_time += this_time;
    if (sv->workloads[w_idx].maxFreq){
      this_freq_power = 1050;
    } else{
      this_freq_power = 450;
    }

    est_pow += (((double)(this_time)/(double)(sec))
                * this_freq_power);
  }

  printf("Est Power: %f mW.\nEst Run Time: %lld\xC2\xB5s.\n\n", est_pow, est_time/2);

  if (time >= sec){
    sv->schedule_feasible = false;
  }
  update_SV_avg(sv, pow, time);
}


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

/////////////////////////////////////////////////////////////////////////////
// From here, We provide functions related to Sample 3.
/////////////////////////////////////////////////////////////////////////////

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
