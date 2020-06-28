#include <string.h>
#include <stdio.h>

#include "map.h"

int main(int argc, char **argv) {
	struct map map;

	memset(&map, 0, sizeof(map));

	for (argv++; *argv != NULL; argv++)
		if (map_set(&map, *argv, "abra") < 0)
			return 1;

	fprintf(stdout, ".");

	return 0;
}
