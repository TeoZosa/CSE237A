// UCSD CSE237A - WI18
// All right reserved

// Important! DO NOT MODIFY this file. You will not submit this file.
// You need to characterize the workloads given with this file.
// For more details, please see the instructions in the class website.

// workload.h: Characterized workload

#ifndef _WORKLOAD1_H_
#define _WORKLOAD1_H_

// First workload - implemented in workload_known.c
void* workload1_init(void*);
void* workload1_body(void*);
void* workload1_exit(void*);

// Second workload - implemented in workload_known.c
void* workload2_init(void*);
void* workload2_body(void*);
void* workload2_exit(void*);

// Third workload - implemented in workload_unknown.o
// The source code of this workload is not given.
void* workload3_init(void*);
void* workload3_body(void*);
void* workload3_exit(void*);

#endif
