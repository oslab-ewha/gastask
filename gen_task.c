#include "gasgen.h"

unsigned	wcet_min, wcet_max, mem_total;
double		util_cpu, util_target;
unsigned	n_tasks_target;

static double	util_sum_cpu, util_cpu_1task;
static unsigned	memreq_1task;
static unsigned	memreq_total;

static void
do_gen_task(FILE *fp)
{
	unsigned	wcet;
	unsigned	duration, memreq;
	double		mem_active_ratio;

	wcet = wcet_min + get_rand(wcet_max - wcet_min + 1);
	duration = (unsigned)(wcet / util_cpu_1task);
	duration = duration + (int)get_rand(duration / 2) - (int)get_rand(duration / 2);

	memreq = memreq_1task + (int)get_rand(memreq_1task / 2) - (int)get_rand(memreq_1task / 2);
	mem_active_ratio = 0.1 + get_rand(1000) / 10000.0 - get_rand(1000) / 10000.0;

	util_sum_cpu += ((double)wcet / duration);
	memreq_total += memreq;

	fprintf(fp, "%u %u %u %lf\n", wcet, duration, memreq, mem_active_ratio);
}

static double
get_mem_util(void)
{
	double	util_bymem = util_target - util_cpu;
	double	util_mem = 1.0 / n_mems;
	int	i;

	for (i = 1; i < n_mems && util_bymem > 0; i++) {
		double	util_overhead = 1 / mems[i].wcet_scale - 1;

		if (util_overhead < util_bymem) {
			util_bymem -= util_overhead;
			util_mem += 1.0 / n_mems;
		}
		else {
			util_mem += 1.0 / n_mems * util_bymem / util_overhead;
			break;
		}
	}

	return util_mem;
}

static double
get_util_overhead_bymem(unsigned mem_used)
{
	double	util_overhead = 0;
	unsigned	mem_total_per_type = mem_total / n_mems;
	int	i;

	for (i = 0; i < n_mems; i++) {
		double	overhead = 1 / mems[i].wcet_scale - 1;

		if (mem_used < mem_total_per_type) {
			util_overhead += overhead * mem_used / mem_total_per_type;
			break;
		}
		else {
			util_overhead += overhead;
			mem_used -= mem_total_per_type;
		}
	}

	return util_overhead;
}

void
gen_task(void)
{
	FILE	*fp;
	double	util_mem_1task;
	unsigned	i;

	util_cpu_1task = util_cpu / n_tasks_target;
	util_mem_1task = get_mem_util() / n_tasks_target;
	memreq_1task = mem_total * util_mem_1task;

	fp = fopen("task_generated.txt", "w");
	if (fp == NULL) {
		FATAL(2, "cannot open task_generated.txt");
	}
	for (i = 0; i < n_tasks_target; i++) {
		do_gen_task(fp);
	}
	fclose(fp);

	printf("full power utilization: %lf\n", util_sum_cpu + get_util_overhead_bymem(memreq_total));
}
