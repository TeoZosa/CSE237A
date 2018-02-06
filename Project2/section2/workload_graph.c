// UCSD CSE237A - WI18
// All rights reserved

// Important! DO NOT MODIFY this file. You will not submit this file.
// You can access the workloads to be characterized and executed 
// using the functions implemented here.
// For more details, please see the instructions in the class website.

// workload_graph.c: utilities to access the task graph of the given workloads  

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "workload.h"

// The following variables are defined in "workload.o" which includes all
// the workloads and their dependencies.
// - The source code of the workloads are not provided.
// - You need to characterize those workloads before scheduling.
// See also your assignment2.c.
extern const int num_workloads;
extern const WorkloadItem workload_array[];

// Get the number of workloads
int get_num_workloads() {
    return num_workloads;
}

// Get a single workload
const WorkloadItem* get_workload(int idx) {
    return &workload_array[idx];
}

// This function creates "task_graph.pdf" file for the task graph.
void write_workload_graph() {
    // check if "dot" command is available
    const char* FILENAME_DOT = "task_graph.dot";
    const char* FILENAME_PDF = "task_graph.pdf";

    // Prepare a structure to check terminal tasks
    int idx;
    int* starting_tasks = (int*)malloc(sizeof(int) * num_workloads);
    for (idx = 0; idx < num_workloads; ++idx) {
        starting_tasks[idx] = 1;
    }

    // Write the file
    FILE* fp = fopen(FILENAME_DOT, "w");

    // Dot header
    fprintf(fp, "digraph Tree {\n");
    fprintf(fp, "\tnode [shape=box, style=\"filled, rounded\", "
            "color=\"black\", fontname=helvetica] ;\n");
    fprintf(fp, "\tedge [fontname=helvetica] ;\n");

    // Start & end node
    fprintf(fp, "\t0 [label=<START>, fillcolor=\"#FFF0F5\"]\n");
    fprintf(fp, "\t%d [label=<END>, fillcolor=\"#FFF0F5\"]\n", num_workloads+1);

    // Draw all the task nodes
    for (idx = 0; idx < num_workloads; ++idx) {
        fprintf(fp, "\t%d [label=<idx = %d>, fillcolor=\"#FFFFFF\"]\n",
                idx+1, idx);
    }

    // Draw the edges between workloads
    for (idx = 0; idx < num_workloads; ++idx) {
        const WorkloadItem* item = get_workload(idx);
        if (item->successor_idx == NULL_TASK)
            continue;

        fprintf(fp, "\t%d -> %d ;\n", idx+1, item->successor_idx+1);

        // If a node is the successor of someone, it's not a starting task
        starting_tasks[item->successor_idx] = 0;
    }

    // Draw edges for the start nodes
    for (idx = 0; idx < num_workloads; ++idx) {
        if (starting_tasks[idx] == 0)
            continue;

        fprintf(fp, "\t0 -> %d [style=dashed];\n", idx+1);
    }

    // Draw edges for the terminal nodes
    for (idx = 0; idx < num_workloads; ++idx) {
        const WorkloadItem* item = get_workload(idx);
        if (item->successor_idx != NULL_TASK)
            continue;

        fprintf(fp, "\t%d -> %d [style=dashed];\n", idx+1, num_workloads+1);
    }
    
    // Draw a loop edge from start to end
    fprintf(fp, "\t%d -> 0 [style=dashed];\n", num_workloads+1);

    // Close the file
    fprintf(fp, "}\n");
    fclose(fp);
    free(starting_tasks);

    // Try to create PDF
    if (access("/usr/bin/dot", X_OK) != 0) {
        printf("Can not create %s\n", FILENAME_PDF);
        printf(
                "Have you installed graphviz?"
                "($ sudo apt-get install graphviz)\n");
        printf("The task graph is written instead to %s\n", FILENAME_DOT);
        return;
    } 

    char cmd[256];
    sprintf(cmd, "/usr/bin/dot -Tpdf %s -o %s", FILENAME_DOT, FILENAME_PDF);
    system(cmd);
    printf("The task graph is written to %s\n", FILENAME_PDF);
}
