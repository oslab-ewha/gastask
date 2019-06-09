#include "gasgen.h"

unsigned	wcet_min, wcet_max, mem_total;
double		mem_power, util_cpu, util_mem;
unsigned	n_tasks_target;

static double	util_cpu_1task;
static unsigned	memreq_1task;

static void
do_gen_task(FILE *fp)
{
	unsigned	wcet;
	unsigned	duration, memreq;
	double		mem_active_ratio;

	wcet = wcet_min + get_rand(wcet_max - wcet_min + 1);
	duration = (unsigned)(wcet / util_cpu_1task);
	duration = duration + (int)get_rand(duration / 10) - (int)get_rand(duration / 10);

	memreq = memreq_1task + (int)get_rand(memreq_1task / 4) - (int)get_rand(memreq_1task / 4);
	mem_active_ratio = mem_power / memreq;

	mem_active_ratio += ((int)(get_rand(5000) - (int)get_rand(5000))/ 10000.0 * mem_active_ratio);

	fprintf(fp, "%u %u %u %lf\n", wcet, duration, memreq, mem_active_ratio);
}

void
gen_task(void)
{
	FILE	*fp;
	double	util_mem_1task;
	unsigned	i;

	util_cpu_1task = util_cpu / n_tasks_target;
	util_mem_1task = util_mem / n_tasks_target;
	memreq_1task = mem_total * util_mem_1task;

	fp = fopen("task_generated.txt", "w");
	if (fp == NULL) {
		FATAL(2, "cannot open task_generated.txt");
	}
	for (i = 0; i < n_tasks_target; i++) {
		do_gen_task(fp);
	}
	fclose(fp);
}
