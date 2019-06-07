#include "gastask.h"

typedef enum {
	SECT_UNKNOWN,
	SECT_GENETIC,
	SECT_CPUFREQ,
	SECT_MEM,
	SECT_TASK,
} section_t;

static char *
trim(char *str)
{
	char	*p;

	if (str == NULL)
		return NULL;

	for (p = str; *p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'; p++);
	str = p;
	if (*p != '\0') {
		for (p = p + 1; *p != '\0'; p++);
		for (p = p - 1; (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') && p != str; p--);
		p[1] = '\0';
	}

	return str;
}

static section_t
check_section(const char *line)
{
	if (*line != '*')
		return FALSE;
	if (strncmp(line + 1, "genetic", 7) == 0)
		return SECT_GENETIC;
	if (strncmp(line + 1, "cpufreq", 7) == 0)
		return SECT_CPUFREQ;
	if (strncmp(line + 1, "mem", 3) == 0)
		return SECT_MEM;
	if (strncmp(line + 1, "task", 4) == 0)
		return SECT_TASK;
	return SECT_UNKNOWN;
}

static void
parse_genetic(FILE *fp)
{
	char	buf[1024];

	while (fgets(buf, 1024, fp)) {
		if (buf[0] == '#')
			continue;
		if (buf[0] == '\n' || buf[0] == '*') {
			fseek(fp, -1 * strlen(buf), SEEK_CUR);
			return;
		}
		if (sscanf(buf, "%u %u %lf %lf", &max_gen, &n_pops, &cutoff, &penalty) != 4) {
			FATAL(2, "cannot load configuration: invalid genetic parameters: %s", trim(buf));
		}
	}
}

static void
parse_cpufreq(FILE *fp)
{
	char	buf[1024];

	while (fgets(buf, 1024, fp)) {
		double	wcet_scale, power_active, power_idle;

		if (buf[0] == '#')
			continue;
		if (buf[0] == '\n' || buf[0] == '*') {
			fseek(fp, -1 * strlen(buf), SEEK_CUR);
			return;
		}
		if (sscanf(buf, "%lf %lf %lf", &wcet_scale, &power_active, &power_idle) != 3) {
			FATAL(2, "cannot load configuration: invalid CPU frequency format: %s", trim(buf));
		}

		if (wcet_scale < 0 || wcet_scale > 1) {
			FATAL(2, "invalid cpu frequency wcet scale: %s", trim(buf));
		}
		if (power_active < 0 || power_idle < 0) {
			FATAL(2, "invalid cpu frequency power: %s", trim(buf));
		}
		add_cpufreq(wcet_scale, power_active, power_idle);
	}	
}

static void
parse_mem(FILE *fp)
{
	char	buf[1024];

	while (fgets(buf, 1024, fp)) {
		unsigned	max_capacity;
		double		wcet_scale, power_active, power_idle;
		char		type[1024];

		if (buf[0] == '#')
			continue;
		if (buf[0] == '\n' || buf[0] == '*') {
			fseek(fp, -1 * strlen(buf), SEEK_CUR);
			return;
		}
		if (sscanf(buf, "%s %u %lf %lf %lf", type, &max_capacity, &wcet_scale, &power_active, &power_idle) != 5) {
			FATAL(2, "cannot load configuration: invalid memory spec: %s", trim(buf));
		}

		if (max_capacity == 0) {
			FATAL(2, "invalid max memory capacity: %s", trim(buf));
		}
		if (wcet_scale < 0 || wcet_scale > 1) {
			FATAL(2, "invalid memory wcet scale: %s", trim(buf));
		}
		if (power_active < 0 || power_idle < 0) {
			FATAL(2, "invalid memory power: %s", trim(buf));
		}

		add_mem(type, max_capacity, wcet_scale, power_active, power_idle);
	}
}

static void
parse_task(FILE *fp)
{
	char	buf[1024];

	while (fgets(buf, 1024, fp)) {
		unsigned	wcet, period, memreq;
		double		mem_active_ratio;

		if (buf[0] == '#')
			continue;
		if (buf[0] == '\n' || buf[0] == '*') {
			fseek(fp, -1 * strlen(buf), SEEK_CUR);
			return;
		}
		if (sscanf(buf, "%u %u %u %lf", &wcet, &period, &memreq, &mem_active_ratio) != 4) {
			FATAL(2, "cannot load configuration: invalid task format: %s", trim(buf));
		}

		if (wcet >= period) {
			FATAL(2, "wcet is larger or equal than period: %s", trim(buf));
		}
		add_task(wcet, period, memreq, mem_active_ratio);
	}
}

static void
parse_conf(FILE *fp)
{
	char	buf[1024];

	while (fgets(buf, 1024, fp)) {
		if (buf[0] == '\n' || buf[0] == '#')
			continue;
		switch (check_section(buf)) {
		case SECT_GENETIC:
			parse_genetic(fp);
			break;
		case SECT_CPUFREQ:
			parse_cpufreq(fp);
			break;
		case SECT_MEM:
			parse_mem(fp);
			break;
		case SECT_TASK:
			if (n_cpufreqs == 0) {
				FATAL(2, "cpu frequency section should be defined ahead of task section");
			}
			parse_task(fp);
			break;
		default:
			errmsg("unknown section: %s", trim(buf));
			FATAL(2, "cannot load configuration");
		}
	}
}

void
load_conf(const char *fpath)
{
	FILE	*fp;

	fp = fopen(fpath, "r");
	if (fp == NULL) {
		FATAL(1, "configuration not found: %s", fpath);
	}

	parse_conf(fp);

	fclose(fp);
}
