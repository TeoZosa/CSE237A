// UCSD CSE237A - WI18
// All rights reserved

// Important! DO NOT MODIFY this file. You will not submit this file.
// This file is provided for your convenience to change the CPU frequency
// in the workload analysis.
// For more details, please see the instructions in the class website.

// cpufreq.h: CPU frequency interface

#ifndef _CPU_FREQ_H_
#define _CPU_FREQ_H_

void set_userspace_governor();
void set_ondemand_governor();
void set_by_max_freq();
void set_by_min_freq();
int get_cur_freq();

#endif

