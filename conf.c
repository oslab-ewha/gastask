#include "gastask.h"

char *
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

section_t
check_section(const char *line)
{
	if (*line != '*')
		return FALSE;
	if (strncmp(line + 1, "genetic", 7) == 0)
		return SECT_GENETIC;
	if (strncmp(line + 1, "gentask", 7) == 0)
		return SECT_GENTASK;
	if (strncmp(line + 1, "cpufreq", 7) == 0)
		return SECT_CPUFREQ;
	if (strncmp(line + 1, "mem", 3) == 0)
		return SECT_MEM;
	if (strncmp(line + 1, "task", 4) == 0)
		return SECT_TASK;
	return SECT_UNKNOWN;
}

void
skip_section(FILE *fp)
{
	char	buf[1024];

	while (fgets(buf, 1024, fp)) {
		if (buf[0] == '#')
			continue;
		if (buf[0] == '\n' || buf[0] == '*') {
			fseek(fp, -1 * strlen(buf), SEEK_CUR);
			return;
		}
	}
}

void
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
