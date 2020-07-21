#include "gastask.h"

mem_t		mems[MAX_MEMS];
unsigned	n_mems;

void
add_mem(const char *typestr, unsigned max_capacity, double wcet_scale, double power_active, double power_idle)
{
	mem_t	*mem;

	if (n_mems >= MAX_MEMS) {
		FATAL(2, "too many memory types");
	}

	mem = &mems[n_mems];
	mem->typestr = strdup(typestr);
	mem->wcet_scale = wcet_scale;
	mem->max_capacity = max_capacity;
	mem->power_active = power_active;
	mem->power_idle = power_idle;
	n_mems++;
}
