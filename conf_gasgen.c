#include "gastask.h"

extern unsigned	wcet_min, wcet_max, mem_total;
extern double	util_cpu, util_target;
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
		if (sscanf(buf, "%u %u %u %lf %lf %u", &wcet_min, &wcet_max, &mem_total,
			   &util_cpu, &util_target, &n_tasks_target) != 6) {
			FATAL(2, "cannot load configuration: invalid gentask parameters: %s", trim(buf));
		}
		if (util_cpu > util_target) {
			FATAL(2, "target utilization cannot be smaller than full utilzation");
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
		case SECT_TASK:
			skip_section(fp);
			break;
		case SECT_MEM:
			parse_mem(fp);
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
