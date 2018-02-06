// UCSD CSE237A - WI18
// All rights reserved

// Important! DO NOT MODIFY this file. You will not submit this file.
// This file is provided for your understanding of the program procedure.
// For more details, please see the instructions in the class website

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <wiringPi.h>
#include <assert.h>

#include "section1.h"
#include "section2.h"

#include "shared_var.h"
#include "cpufreq.h"
#include "scheduler.h"

#include "workload.h"
#include "workload_util.h"

int main(int argc, char* argv[]) {
	SharedVariable v;
    int running_time_in_sec = 10;
    if (argc == 2) {
        running_time_in_sec = atoi(argv[1]);
    }

	if (wiringPiSetup() == -1) {
		printf("Failed to setup wiringPi.");
		return 1; 
	}

    // Comment the following function if you don't want to save
    // the task graph of the workloads to "task_graph.pdf"
    write_workload_graph();

    // Initialize the sensor program (implemented in Section 1) and governor
	program_init(&v);
    set_userspace_governor();

    // Learn the workload: You need to implement your code in learn_workloads().
    // Make sure that learn_workloads() function takes less than 300 seconds.
    // See "assignment2.c"
    TimeType start_learn_time = get_current_time_us();
    learn_workloads(&v);
    TimeType elapsed_learning_time = get_current_time_us() - start_learn_time;
    printf("Your learn_workloads() function takes %d secs.\n",
            (int)(elapsed_learning_time / (1000 * 1000)));
    if (elapsed_learning_time > 300 * 1000 * 1000) {
        printf("[ERROR] Your learn_workloads() function has to be executed "
                "within 300 secs.\n");
        assert(false);
    }

    // Initialize the sheduler
    init_scheduler(&v);
    unregister_workload_all(); // Unregister all workload in profiling (if any)
    set_userspace_governor(); // Change the governor (in case if needed)
    
    // Do scheduling
    printf("Start scheduling.\n");
    TimeType start_sched_time = get_current_time_us();
    while ((v.bProgramExit != 1) && 
            ((get_current_time_us() - start_sched_time) <
            running_time_in_sec * 1000 * 1000)) {
        start_scheduling(&v);
        schedule();
        finish_scheduling(&v);
    }

    // exit scheduling
    exit_scheduler();
    set_ondemand_governor();
    program_exit(&v);

	printf("Program finished.\n");

	return 0;
}
