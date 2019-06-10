#include "gastask.h"

extern unsigned	wcet_min, wcet_max, mem_total;
extern double	mem_power, util_cpu, util_mem;
extern unsigned	n_tasks_target;

static void
parse_gentask(FILE *fp)
{
	char	buf[1024];

	while (fgets(buf, 1024, fp)) {
		if (buf[0] == '#')
			continue;
		if (buf[0] == '\n' || buf[0] == '*') {
			fseek(fp, -1 * strlen(buf), SEEK_CUR);
			return;
		}
		if (sscanf(buf, "%u %u %u %lf %lf %lf %u", &wcet_min, &wcet_max, &mem_total,
			   &mem_power, &util_cpu, &util_mem, &n_tasks_target) != 7) {
			FATAL(2, "cannot load configuration: invalid gentask parameters: %s", trim(buf));
		}
	}
}

void
parse_conf(FILE *fp)
{
	char	buf[1024];

	while (fgets(buf, 1024, fp)) {
		if (buf[0] == '\n' || buf[0] == '#')
			continue;
		switch (check_section(buf)) {
		case SECT_GENETIC:
		case SECT_CPUFREQ:
		case SECT_MEM:
		case SECT_TASK:
			skip_section(fp);
			break;
		case SECT_GENTASK:
			parse_gentask(fp);
			break;
		default:
			errmsg("unknown section: %s", trim(buf));
			FATAL(2, "cannot load configuration");
		}
	}
}
