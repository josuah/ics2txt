#ifndef MAP_H
#define MAP_H

#include <stddef.h>

struct map_entry {
	char *key;
	void *value;
};

struct map {
	struct map_entry *entry;
	size_t len;
};

/** src/map.c **/
void * map_get(struct map *map, char *key);
int map_set(struct map *map, char *key, void *value);
int map_del(struct map *map, char *key);
void map_init(struct map *map);
void map_free_values(struct map *map);
void map_free(struct map *map);

#endif
