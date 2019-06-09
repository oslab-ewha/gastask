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
