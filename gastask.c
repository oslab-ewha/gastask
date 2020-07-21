#include "gastask.h"

BOOL	verbose;

static int	seed = 0;

static void
usage(void)
{
	fprintf(stdout,
"Usage: gastask <options> <config path>\n"
" <options>\n"
"      -h: this message\n"
"      -v: verbose mode\n"
"      -s <seed>: (default: 0)\n"
	);
}

void
errmsg(const char *fmt, ...)
{
	va_list	ap;
	char	*errmsg;

	va_start(ap, fmt);
	vasprintf(&errmsg, fmt, ap);
	va_end(ap);

	fprintf(stderr, "ERROR: %s\n", errmsg);

	free(errmsg);
}

static void
parse_args(int argc, char *argv[])
{
	int	c;

	while ((c = getopt(argc, argv, "s:h")) != -1) {
		switch (c) {
		case 's':
			if (sscanf(optarg, "%d", &seed) != 1) {
				usage();
				exit(1);
			}
			break;
		case 'h':
			usage();
			exit(0);
		default:
			errmsg("invalid option");
			usage();
			exit(1);
		}
	}

	if (argc - optind < 1) {
		usage();
		exit(1);
	}

	load_conf(argv[optind]);
}

int
main(int argc, char *argv[])
{
	parse_args(argc, argv);
	srand(seed);
	
	run_GA();

	return 0;
}
