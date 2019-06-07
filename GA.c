#include "gastask.h"

#define MAX_TRY	100

unsigned	n_pops = 100;
unsigned	max_gen = 100000;

double		cutoff, penalty;

LIST_HEAD(genes_by_util);
LIST_HEAD(genes_by_power);
LIST_HEAD(genes_by_score);

gene_t	*genes;

static unsigned
get_rand(unsigned max_value)
{
	return abs(rand()) % max_value;
}

static void
assign_task_values(unsigned char *task_values, unsigned max_value)
{
	int	i;

	for (i = 0; i < n_tasks; i++)
		task_values[i] = get_rand(max_value);
}

static void
sort_gene_util(gene_t *gene)
{
	struct list_head	*lp;

	list_for_each (lp, &genes_by_util) {
		gene_t	*gene_list = list_entry(lp, gene_t, list_util);
		if (gene->util < gene_list->util) {
			list_add_tail(&gene->list_util, &gene_list->list_util);
			return;
		}
	}
	list_add_tail(&gene->list_util, &genes_by_util);
}

static void
sort_gene_power(gene_t *gene)
{
	struct list_head	*lp;

	list_for_each (lp, &genes_by_power) {
		gene_t	*gene_list = list_entry(lp, gene_t, list_power);
		if (gene->power < gene_list->power) {
			list_add_tail(&gene->list_power, &gene_list->list_power);
			return;
		}
	}
	list_add_tail(&gene->list_power, &genes_by_power);
}

static void
sort_gene_score(gene_t *gene)
{
	struct list_head	*lp;

	list_for_each (lp, &genes_by_score) {
		gene_t	*gene_list = list_entry(lp, gene_t, list_score);
		if (gene->score < gene_list->score) {
			list_add_tail(&gene->list_score, &gene_list->list_score);
			return;
		}
	}
	list_add_tail(&gene->list_score, &genes_by_score);
}

static void
sort_gene(gene_t *gene)
{
	sort_gene_util(gene);
	sort_gene_power(gene);
	sort_gene_score(gene);
}

static BOOL
calc_utilpower(gene_t *gene)
{
	double	util_new = 0, power_new = 0;
	int	i;

	for (i = 0; i < n_tasks; i++) {
		double	task_util, task_power;
		
		get_task_utilpower(i, gene->mems[i], gene->cpufreqs[i], &task_util, &task_power);
		util_new += task_util;
		power_new += task_power;
	}
	if (util_new <= cutoff) {
		gene->util = util_new;
		gene->power = power_new;
		gene->score = power_new;
		if (util_new >= 1.0)
			gene->score += power_new * penalty;
		list_del_init(&gene->list_util);
		list_del_init(&gene->list_power);
		list_del_init(&gene->list_score);
		sort_gene(gene);
		return TRUE;
	}
	return FALSE;
}

static void
init_gene(gene_t *gene)
{
	int	i;

	for (i = 0; i < MAX_TRY; i++) {
		assign_task_values(gene->mems, n_mems);
		assign_task_values(gene->cpufreqs, n_mems);

		INIT_LIST_HEAD(&gene->list_util);
		INIT_LIST_HEAD(&gene->list_power);
		INIT_LIST_HEAD(&gene->list_score);

		if (calc_utilpower(gene))
			break;
	}
	if (i == MAX_TRY)
		FATAL(3, "cannot generate initial genes");
}

static void
init_populations(void)
{
	gene_t	*gene;
	int	i;

	gene = genes = (gene_t *)malloc(n_pops * sizeof(gene_t));

	for (i = 0; i < n_pops; i++, gene++) {
		init_gene(gene);
	}
}

static void
swap_values(unsigned char values1[], unsigned char values2[], unsigned crosspt)
{
	unsigned char	values_temp[MAX_TASKS];

	memcpy(values_temp, values1, crosspt);

	memcpy(values1, values2, crosspt);
	memcpy(values2, values_temp, crosspt);
}

static BOOL
do_crossover(unsigned n1, unsigned n2, unsigned crosspt_mem, unsigned crosspt_cpufreq)
{
	gene_t	*gene1 = genes + n1, *gene2 = genes + n2;

	swap_values(gene1->mems, gene2->mems, crosspt_mem);
	swap_values(gene1->cpufreqs, gene2->cpufreqs, crosspt_cpufreq);

	if (calc_utilpower(gene1) && calc_utilpower(gene2))
		return TRUE;

	swap_values(gene2->mems, gene1->mems, crosspt_mem);
	swap_values(gene2->cpufreqs, gene1->cpufreqs, crosspt_cpufreq);
	calc_utilpower(gene1);
	calc_utilpower(gene2);

	return FALSE;
}

static void
crossover(void)
{
	int	i;

	for (i = 0; i < MAX_TRY; i++) {
		unsigned	n1, n2;
		unsigned	crosspt_mem, crosspt_cpufreq;
	
		n1 = get_rand(n_pops);
		do {
			n2 = get_rand(n_pops);
		} while (n2 == n1);

		crosspt_mem = get_rand(n_tasks - 1) + 1;
		crosspt_cpufreq = get_rand(n_tasks - 1) + 1;
		if (do_crossover(n1, n2, crosspt_mem, crosspt_cpufreq))
			break;
	}
	if (i == MAX_TRY) {
		FATAL(3, "cannot execute crossover");
	}
}

static void
newborn(void)
{
	gene_t	*gene = list_entry(genes_by_score.prev, gene_t, list_score);

	list_del(&gene->list_util);
	list_del(&gene->list_power);
	list_del(&gene->list_score);

	init_gene(gene);
}

void
run_GA(void)
{
	unsigned	gen = 1;
	
	init_report();
	init_populations();

	add_report(gen);
	
	while (gen <= max_gen) {
		crossover();
		newborn();
		add_report(gen);
		gen++;
	}
	close_report();
}
