all: w1 w2 w3

w1:
	gcc -o main_section2_w1 main_section2.c assignment1.c assignment2.c cpufreq.c scheduler.o 1/workload.o workload_util.c pmu_reader.c workload_graph.c -lwiringPi -lpthread -D_GNU_SOURCE 

w2:
	gcc -o main_section2_w2 main_section2.c assignment1.c assignment2.c cpufreq.c scheduler.o 2/workload.o workload_util.c pmu_reader.c workload_graph.c -lwiringPi -lpthread -D_GNU_SOURCE

w3:
	gcc -o main_section2_w3 main_section2.c assignment1.c assignment2.c cpufreq.c scheduler.o 3/workload.o workload_util.c pmu_reader.c workload_graph.c -lwiringPi -lpthread -D_GNU_SOURCE

clean:
	rm -rf main_section2_w1 main_section2_w2 main_section2_w3
