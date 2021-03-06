// UCSD CSE237A - WI18
// Important! You WILL SUBMIT this file.
#ifndef _SHARED_VAR_H_
#define _SHARED_VAR_H_

#include <stdbool.h>
#include "workload_util.h"

#define NUM_WORKLOADS 16

typedef struct
{    int wl;
		int time;
    int crit_time;
  int maxFreq;

  //TODO: optimize further by adding preferred core and freq?
  //Sounds like diminishing returns

} WLxTime;

static int compare_exec_time(const void *workload1, const void *workload2)
{
	return  ((WLxTime *)workload1)->time - ((WLxTime *)workload2)->time;
}
static int compare_crit_time(const void *workload1, const void *workload2)
{
  return  ((WLxTime *)workload1)->crit_time - ((WLxTime *)workload2)->crit_time;
}

// **** Shared structure *****
// All thread fuctions get a shared variable of the structure
// as the function parameter.
// If needed, you can add anything in this structure.
typedef struct shared_variable {
	int bProgramExit; // Once it is set to 1, the program will be terminated.
	int running;
	int smallOn;
	int bigOn;
	long long start_time;

  unsigned short int scheduledWorkloads[NUM_WORKLOADS];
	WLxTime workloads[NUM_WORKLOADS];
  bool no_best_schedule_yet;
  bool schedule_feasible; // if schedule ever over 1 s, don't choose it; can't risk it.

//  below: different variables we are optimizing for
  unsigned short int times_run_curr_schedule; // for a configuration to average the times
  double avg_pow_curr_schedule;
  double avg_time_curr_schedule;

  bool is_max_freq;
  bool is_max_freq_best;

  bool is_exec_time;
  bool is_exec_time_best;

  double best_pow;
  double best_time;
  WLxTime workloads_best_ordering[NUM_WORKLOADS];


} SharedVariable;

#endif
