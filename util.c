#include "common.h"

unsigned
get_rand(unsigned max_value)
{
	return abs(rand()) % max_value;
}
