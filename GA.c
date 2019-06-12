#include "gastask.h"

#define MAX_TRY	10000

unsigned	n_pops = 100;
unsigned	max_gen = 100000;

double		cutoff, penalty;

LIST_HEAD(genes_by_util);
LIST_HEAD(genes_by_power);
LIST_HEAD(genes_by_score);

gene_t	*genes;

#if 0

static void
show_gene(gene_t *gene)
{
	int	i;

	printf("power: %.2lf mem:", gene->power);
	for (i = 0; i < n_tasks; i++) {
		printf("%c", gene->mems[i] + '0');
	}
	printf(" cpufreq:");
	for (i = 0; i < n_tasks; i++) {
		printf("%c", gene->cpufreqs[i] + '0');
	}
	printf("\n");
}

#endif

static void
setup_taskattrs(taskattrs_t *taskattrs)
{
	int	i;

	memset(taskattrs->n_tasks_per_type, 0, MAX_ATTRTYPES);
	taskattrs->max_type = 0;

	for (i = 0; i < n_tasks; i++) {
		unsigned	attrtype = taskattrs->attrs[i];
		taskattrs->n_tasks_per_type[attrtype]++;
		if (taskattrs->n_tasks_per_type[taskattrs->max_type] < taskattrs->n_tasks_per_type[attrtype]) {
			taskattrs->max_type = attrtype;
		}
	}
}

static void
assign_taskattrs(taskattrs_t *taskattrs, unsigned max_value)
{
	int	i;

	for (i = 0; i < n_tasks; i++) {
		unsigned	attrtype = get_rand(max_value);
		taskattrs->attrs[i] = attrtype;
	}
	setup_taskattrs(taskattrs);
}

static void
sort_gene_util(gene_t *gene)
{
	struct list_head	*lp;

	list_del_init(&gene->list_util);

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

	list_del_init(&gene->list_power);

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

	list_del_init(&gene->list_score);

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
check_memusage(gene_t *gene)
{
	unsigned	mem_used[MAX_MEMS] = { 0, };
	int	i;

	for (i = 0; i < n_tasks; i++) {
		mem_used[gene->taskattrs_mem.attrs[i]] += get_task_memreq(i);
	}
	for (i = 0; i < n_mems; i++) {
		if (mem_used[i] > mems[i].max_capacity)
			return FALSE;
	}
	return TRUE;
}

static void
balance_mem_types(gene_t *gene)
{
	taskattrs_t	*taskattrs = &gene->taskattrs_mem;
	unsigned	n_tasks_type;

	n_tasks_type = taskattrs->n_tasks_per_type[taskattrs->max_type];
	if (n_tasks_type > 0) {
		unsigned	idx_changed = get_rand(n_tasks_type);
		unsigned	type_new;
		unsigned	i;
		
		for (i = 0; i < n_tasks; i++) {
			if (taskattrs->attrs[i] == taskattrs->max_type) {
				if (idx_changed == 0)
					break;
				idx_changed--;
			}
		}
		type_new = get_rand_except(n_mems, taskattrs->max_type);
		taskattrs->attrs[i] = type_new;
		taskattrs->n_tasks_per_type[taskattrs->max_type]--;
		taskattrs->n_tasks_per_type[type_new]++;
		if (taskattrs->n_tasks_per_type[taskattrs->max_type] < taskattrs->n_tasks_per_type[type_new]) {
			taskattrs->max_type = type_new;
		}
		
	}
}

static BOOL
lower_utilization_by_attr(taskattrs_t *taskattrs)
{
	unsigned	idx_org, idx;
	
	idx_org = idx = get_rand(n_tasks);
	do {
		unsigned	type = taskattrs->attrs[idx];
		if (type > 0) {
			taskattrs->attrs[idx]--;
			
			taskattrs->n_tasks_per_type[type]--;
			taskattrs->n_tasks_per_type[type - 1]++;
			if (taskattrs->n_tasks_per_type[type - 1] > taskattrs->n_tasks_per_type[taskattrs->max_type])
				taskattrs->max_type = type - 1;
			return TRUE;
		}
		idx++;
		idx %= n_tasks;
	} while (idx != idx_org);
	
	return FALSE;
}

static void
lower_utilization(gene_t *gene)
{
	if (get_rand(n_cpufreqs + n_mems) < n_cpufreqs) {
		if (!lower_utilization_by_attr(&gene->taskattrs_cpufreq))
			lower_utilization_by_attr(&gene->taskattrs_mem);
	}
	else {
		if (!lower_utilization_by_attr(&gene->taskattrs_mem))
			lower_utilization_by_attr(&gene->taskattrs_cpufreq);
	}
}

BOOL
check_utilpower(gene_t *gene)
{
	double	util_new = 0, power_new, power_new_sum_cpu = 0, power_new_sum_mem = 0;
	int	i;

	for (i = 0; i < n_tasks; i++) {
		double	task_util, task_power_cpu, task_power_mem;
		
		get_task_utilpower(i, gene->taskattrs_mem.attrs[i], gene->taskattrs_cpufreq.attrs[i],
				   &task_util, &task_power_cpu, &task_power_mem);
		util_new += task_util;
		power_new_sum_cpu += task_power_cpu;
		power_new_sum_mem += task_power_mem;
	}
	power_new = power_new_sum_cpu + power_new_sum_mem;
	if (util_new < 1.0) {
		power_new += cpufreqs[n_cpufreqs - 1].power_idle * (1 - util_new);
	}
	gene->util = util_new;
	if (util_new <= cutoff) {
		gene->power = power_new;
		gene->score = power_new;
		if (util_new >= 1.0)
			gene->score += power_new * (util_new - 1.0) * penalty;
		
		return TRUE;
	}
	return FALSE;
}

static void
init_gene(gene_t *gene)
{
	int	i;

	assign_taskattrs(&gene->taskattrs_mem, n_mems);
	assign_taskattrs(&gene->taskattrs_cpufreq, n_cpufreqs);

	for (i = 0; i < MAX_TRY; i++) {
		INIT_LIST_HEAD(&gene->list_util);
		INIT_LIST_HEAD(&gene->list_power);
		INIT_LIST_HEAD(&gene->list_score);

		if (!check_memusage(gene)) {
			balance_mem_types(gene);
			continue;
		}
		if (check_utilpower(gene)) {
			sort_gene(gene);
			return;
		}
		lower_utilization(gene);
	}

	FATAL(3, "cannot generate initial genes: utilization too high: %lf", gene->util);
}

static void
init_populations(void)
{
	gene_t	*gene;
	double	util_sum = 0;
	int	i;

	gene = genes = (gene_t *)calloc(n_pops, sizeof(gene_t));

	for (i = 0; i < n_pops; i++, gene++) {
		init_gene(gene);
		util_sum += gene->util;
	}
	printf("initial utilization: %lf\n", util_sum / n_pops);
}

static void
inherit_values(taskattrs_t *taskattrs_newborn, taskattrs_t *taskattrs1, taskattrs_t *taskattrs2, unsigned crosspt)
{
	memcpy(taskattrs_newborn->attrs, taskattrs1->attrs, crosspt);
	memcpy(taskattrs_newborn->attrs + crosspt, taskattrs2->attrs + crosspt, n_tasks - crosspt);
	setup_taskattrs(taskattrs_newborn);
}

static BOOL
do_crossover(gene_t *newborn, gene_t *gene1, gene_t *gene2, unsigned crosspt_mem, unsigned crosspt_cpufreq)
{
	inherit_values(&newborn->taskattrs_mem, &gene1->taskattrs_mem, &gene2->taskattrs_mem, crosspt_mem);
	inherit_values(&newborn->taskattrs_cpufreq, &gene1->taskattrs_cpufreq, &gene2->taskattrs_cpufreq, crosspt_cpufreq);

	if (!check_memusage(newborn))
		return FALSE;
	if (!check_utilpower(newborn))
		return FALSE;
	if (newborn->score > gene1->score || newborn->score > gene2->score)
		return FALSE;
	sort_gene(newborn);
	return TRUE;
}

#define M	4

static unsigned
select_position(void)
{
	unsigned	max = M * (n_pops + 1) / 2;
	unsigned	alpha;
	unsigned	kvalue;

	alpha = get_rand(max);
	kvalue = (sqrt(1 + 8 * alpha * n_pops / M) - 1) / 2;
	ASSERT(n_pops > kvalue);
	return n_pops - kvalue - 1;
}

static gene_t *
select_gene(void)
{
	struct list_head	*lp;
	unsigned	position;

	position = select_position();
	list_for_each (lp, &genes_by_score) {
		gene_t	*gene = list_entry(lp, gene_t, list_score);
		if (position == 0)
			return gene;
		position--;
	}
	return list_entry(genes_by_score.prev, gene_t, list_score);
}

static gene_t *
get_newborn(void)
{
	gene_t	*gene = list_entry(genes_by_score.prev, gene_t, list_score);

	list_del_init(&gene->list_util);
	list_del_init(&gene->list_power);
	list_del_init(&gene->list_score);

	return gene;
}

static void
crossover(void)
{
	gene_t	*newborn;
	int	i;

	newborn = get_newborn();
	for (i = 0; i < MAX_TRY; i++) {
		gene_t	*gene1, *gene2;
		unsigned	crosspt_mem, crosspt_cpufreq;
	
		gene1 = select_gene();
		do {
			gene2 = select_gene();
		} while (gene1 == gene2);

		crosspt_mem = get_rand(n_tasks - 1) + 1;
		crosspt_cpufreq = get_rand(n_tasks - 1) + 1;
		if (do_crossover(newborn, gene1, gene2, crosspt_mem, crosspt_cpufreq))
			break;
	}
	if (i == MAX_TRY) {
		FATAL(3, "cannot execute crossover");
	}
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
		gen++;
		add_report(gen);
	}
	close_report();
}
