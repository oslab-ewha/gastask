#include "common.h"

unsigned
get_rand(unsigned max_value)
{
	return abs(rand()) % max_value;
}

unsigned
get_rand_except(unsigned max_value, unsigned int ex_value)
{
	while (TRUE) {
		unsigned	value = get_rand(max_value);
		if (value != ex_value)
			return value;
	}
}
