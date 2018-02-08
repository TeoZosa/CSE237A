// UCSD CSE237A - WI18
// Important! You WILL SUBMIT this file.
#ifndef _SHARED_VAR_H_
#define _SHARED_VAR_H_

#include "workload_util.h"

#define NUM_WORKLOADS 16

typedef struct
{    int wl;
		int time;
} WLxTime;

static int compare(const void *workload1, const void *workload2)
{
	return  ((WLxTime *)workload1)->time - ((WLxTime *)workload2)->time;
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

} SharedVariable;

#endif
