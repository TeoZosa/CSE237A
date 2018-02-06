// UCSD CSE237A - WI18
// All rights reserved

// Important! DO NOT MODIFY this file. You will not submit this file.
// This file is provided for your convenience to change the CPU frequency
// in the workload analysis.
// For more details, please see the instructions in the class website.

// cpufreq.c: CPU frequency interface

#include "cpufreq.h"
#include <stdio.h>
#include <assert.h>

static int maxFreq = 0;
static int minFreq = 0;

#define POLICY_PATH "/sys/devices/system/cpu/cpufreq/policy0/"

static void set_governor(const char* szNewGovernor) {
	FILE* fp;
	fp = fopen(POLICY_PATH"scaling_governor", "w");
	fprintf(fp, "%s", szNewGovernor);
	fclose(fp);
}

static void read_freq() {
	FILE* fp;
    fp = fopen(POLICY_PATH"scaling_max_freq", "r");
	fscanf(fp, "%d", &maxFreq);
	fclose(fp);

    fp = fopen(POLICY_PATH"scaling_min_freq", "r");
	fscanf(fp, "%d", &minFreq);
	fclose(fp);
}

void set_userspace_governor() {
    set_governor("userspace");
    if (maxFreq == 0)
        read_freq();
}


void set_ondemand_governor() {
    set_governor("ondemand");
    if (maxFreq == 0)
        read_freq();
}

static void set_frequency(int frequency) {
	FILE* fp;
	fp = fopen(POLICY_PATH"scaling_setspeed", "w");
	fprintf(fp, "%d", frequency);
	fclose(fp);
}

void set_by_max_freq() {
    assert(maxFreq > 0);
	set_frequency(maxFreq);
}


void set_by_min_freq() {
    assert(minFreq > 0);
	set_frequency(minFreq);
}

int get_cur_freq() {
	int curFreq;
	FILE* fp = fopen(POLICY_PATH"scaling_setspeed", "r");
	fscanf(fp, "%d", &curFreq);
	fclose(fp);

	return curFreq;
}
