#include "gastask.h"

unsigned	n_tasks;
task_t	tasks[MAX_TASKS];

void
get_task_utilpower(unsigned no_task, unsigned char mem_type, unsigned char cpufreq_type, double *putil, double *ppower_cpu, double *ppower_mem)
{
	task_t	*task = tasks + no_task;
	mem_t	*mem = mems + mem_type;
	cpufreq_t	*cpufreq = cpufreqs + cpufreq_type;
	double	wcet_scaled_cpu = 1 / cpufreq->wcet_scale;
	double	wcet_scaled_mem = 1 / mem->wcet_scale;
	double	cpu_power_unit;
	double	wcet_scaled;

	wcet_scaled = task->wcet * wcet_scaled_cpu * wcet_scaled_mem;
	if (wcet_scaled >= task->period) {
		FATAL(3, "task[%u]: scaled wcet exceeds task period: %lf > %u", task->no, wcet_scaled, task->period);
	}
	*putil = wcet_scaled / task->period;

	cpu_power_unit = (cpufreq->power_active * wcet_scaled_cpu + cpufreq->power_idle * wcet_scaled_mem) / (wcet_scaled_cpu + wcet_scaled_mem);
	*ppower_cpu = cpu_power_unit * wcet_scaled / task->period;
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
