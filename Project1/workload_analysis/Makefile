CC=$(CROSS_COMPILE)gcc
all:
	$(CC) -o main_part1 main.c cpufreq.c pmu_reader.c workload_util.c workload_known.c workload_unknown.o -static -lpthread -D_GNU_SOURCE

clean:
	rm main_part1
