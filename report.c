#include "gastask.h"

#define N_REPORTS	1000

static int	n_report_intervals;

static FILE	*fp;

void
add_report(unsigned gen)
{
	double	util_sum = 0, power_sum = 0;
	double	util_avg, power_avg = -1;
	double	util_min, util_max, power_min = -1, power_max;
	unsigned	n_valid_genes = 0;
	gene_t	*gene;
	struct list_head	*lp;
	int	i;

	if (fp == NULL)
		return;

#ifdef N_REPORTS
	if (n_report_intervals == 0) {
		n_report_intervals = max_gen / N_REPORTS;
		if (n_report_intervals == 0)
			n_report_intervals = 1;
	}
	else if (gen % n_report_intervals != 0 && gen != max_gen)
		return;
#endif
	for (i = 0; i < n_pops; i++) {
		gene = genes + i;
		util_sum += gene->util;
		if (gene->util <= 1.0) {
			power_sum += gene->power;
			n_valid_genes++;
		}
	}

	util_avg = util_sum / n_pops;
	if (n_valid_genes > 0)
		power_avg = power_sum / n_valid_genes;

	gene = list_entry(genes_by_util.next, gene_t, list_util);
	util_min = gene->util;
	gene = list_entry(genes_by_util.prev, gene_t, list_util);
	util_max = gene->util;

	list_for_each (lp, &genes_by_power) {
		gene = list_entry(genes_by_power.next, gene_t, list_power);
		if (gene->util <= 1.0) {
			power_min = gene->power;
			break;
		}
	}
	gene = list_entry(genes_by_power.prev, gene_t, list_power);
	power_max = gene->power;
	if (power_avg < 0)
		power_avg = power_max;
	if (power_min < 0)
		power_min = power_max;
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
	if (gene->util > 1.0) {
		FATAL(2, "over-utilized gene: %lf", gene->util);
	}
	for (i = 0; i < n_tasks; i++) {
		fprintf(fp, "%u %u\n", (unsigned)gene->taskattrs_mem.attrs[i], (unsigned)gene->taskattrs_cpufreq.attrs[i]);
	}
	fclose(fp);

	printf("power: %.3lf util: %.3lf\n", gene->power, gene->util);
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
