#include "gastask.h"

unsigned	n_tasks;
task_t	tasks[MAX_TASKS];

#define MIN(a, b)	((a) < (b) ? (a): (b))

void
get_task_utilpower(unsigned no_task, unsigned char mem_type, unsigned char cpufreq_type, double *putil, double *ppower_cpu, double *ppower_mem)
{
	task_t	*task = tasks + no_task;
	mem_t	*mem = mems + mem_type;
	cpufreq_t	*cpufreq = cpufreqs + cpufreq_type;
	double	wcet_scaled;

	wcet_scaled = (double)task->wcet / MIN(mem->wcet_scale, cpufreq->wcet_scale);
	if (wcet_scaled >= task->period) {
		FATAL(3, "task[%u]: scaled wcet exceeds task period: %lf > %lf", wcet_scaled, task->period);
	}
	*putil = wcet_scaled / task->period;
	*ppower_cpu = cpufreq->power_active * wcet_scaled / task->period +
		cpufreq->power_idle * (1 - wcet_scaled / task->period);
	*ppower_mem = task->memreq * (task->mem_active_ratio * mem->power_active + (1 - task->mem_active_ratio) * mem->power_idle) * wcet_scaled / task->period +
		task->memreq * mem->power_idle * (1 - wcet_scaled / task->period);
}

unsigned
get_task_memreq(unsigned no_task)
{
	task_t	*task = tasks + no_task;

	return task->memreq;
}

void
add_task(unsigned wcet, unsigned period, unsigned memreq, double mem_active_ratio)
{
	task_t	*task;

	task = tasks + n_tasks;
	task->wcet = wcet;
	task->period = period;
	task->memreq = memreq;
	task->mem_active_ratio = mem_active_ratio;

	n_tasks++;
	task->no = n_tasks;
}
