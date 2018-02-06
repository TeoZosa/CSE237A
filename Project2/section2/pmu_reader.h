// UCSD CSE237A - WI18
// All rights reserved

// Important! DO NOT MODIFY this file. You will not submit this file.
// This file is provided for the function declaration of pmu_reader.h.
// You need to change and submit "pmu_reader.c"
// For more details, please see the instructions in the class website.

// pmu_reader.h: PMU Reading interface

#ifndef _PMU_READER_H_
#define _PMU_READER_H_

void reset_counters(void);
unsigned int get_cyclecount(void);
unsigned int get_single_event(unsigned int cnt_index);

// cnt_index is one of the followings:
#define CNT_INST_EXE 0x00
#define CNT_L1D_ACSS 0x01
#define CNT_L1D_MISS 0x02
#define CNT_LLC_ACSS 0x03
#define CNT_LLC_MISS 0x04
#define CNT_TLB_MISS 0x05

#endif
