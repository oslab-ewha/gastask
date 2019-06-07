#include "gastask.h"

static FILE	*fp;

void
add_report(unsigned gen)
{
	double	util_sum = 0, power_sum = 0;
	double	util_avg, power_avg;
	double	util_min, util_max, power_min, power_max;
	gene_t	*gene;
	int	i;

	if (fp == NULL)
		return;

	for (i = 0; i < n_pops; i++) {
		gene = genes + i;
		util_sum += gene->util;
		power_sum += gene->power;
	}

	util_avg = util_sum / n_pops;
	power_avg = power_sum / n_pops;

	gene = list_entry(genes_by_util.next, gene_t, list_util);
	util_min = gene->util;
	gene = list_entry(genes_by_util.prev, gene_t, list_util);
	util_max = gene->util;

	gene = list_entry(genes_by_power.next, gene_t, list_power);
	power_min = gene->power;
	gene = list_entry(genes_by_power.prev, gene_t, list_power);
	power_max = gene->power;
	
	fprintf(fp, "%u %lf %lf %lf %lf %lf %lf\n", gen,
		power_min, power_avg, power_max, util_min, util_avg, util_max);
}

static void
save_task_infos(void)
{
	gene_t	*gene;
	int	i;

	fp = fopen("task.txt", "w");
	if (fp == NULL){
		FATAL(2, "cannot open task.txt");
	}

	fprintf(fp, "# mem_idx cpufreq_idx\n");
	gene = list_entry(genes_by_power.next, gene_t, list_power);
	for (i = 0; i < n_tasks; i++) {
		fprintf(fp, "%u %u\n", (unsigned)gene->mems[i], (unsigned)gene->cpufreqs[i]);
	}
	fclose(fp);
}

void
init_report(void)
{
	fp = fopen("report.txt", "w");
	if (fp == NULL) {
		FATAL(2, "cannot open report.txt");
	}
	fprintf(fp, "# generation power_min power_avg power_max util_min util_avg util_max\n");
}

void
close_report(void)
{
	if (fp != NULL)
		fclose(fp);
	save_task_infos();
}
