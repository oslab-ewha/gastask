#include "gastask.h"

cpufreq_t	cpufreqs[MAX_CPU_FREQS];
unsigned	n_cpufreqs;

void
add_cpufreq(double wcet_scale, double power_active, double power_idle)
{
	if (n_cpufreqs >= MAX_CPU_FREQS)
		FATAL(2, "too many cpu frequencies");

	if (n_cpufreqs > 0 && cpufreqs[n_cpufreqs - 1].wcet_scale < wcet_scale)
		FATAL(2, "cpu frequency should be defined in decreasing order");

	n_cpufreqs++;
	cpufreqs[n_cpufreqs - 1].wcet_scale = wcet_scale;
	cpufreqs[n_cpufreqs - 1].power_active = power_active;
	cpufreqs[n_cpufreqs - 1].power_idle = power_idle;
}
