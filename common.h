#ifndef _COMMON_H_
#define _COMMON_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

#include "ecm_list.h"

#define TRUE	1
#define FALSE	0

#define ASSERT(cond)			do { assert(cond); } while (0)
#define FATAL(exitcode, fmt, ...)	do { errmsg(fmt, ## __VA_ARGS__); exit(exitcode); } while (0)

typedef int	BOOL;

typedef enum {
	SECT_UNKNOWN,
	SECT_GENETIC,
	SECT_GENTASK,
	SECT_CPUFREQ,
	SECT_MEM,
	SECT_TASK,
} section_t;

typedef struct {
	char		*typestr;
	unsigned	max_capacity;
	double		wcet_scale;
	double		power_active, power_idle;	/* per tick * mem_req */
} mem_t;

extern unsigned	n_mems;
extern mem_t	mems[];

void errmsg(const char *fmt, ...);
void load_conf(const char *fpath);
void parse_mem(FILE *fp);

section_t check_section(const char *line);
void skip_section(FILE *fp);
void parse_conf(FILE *fp);
char *trim(char *str);
unsigned get_rand(unsigned max_value);
unsigned get_rand_except(unsigned max_value, unsigned ex_value);

#endif
