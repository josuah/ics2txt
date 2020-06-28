#include "map.h"

#include <stdlib.h>
#include <string.h>

#include "util.h"

static int
map_cmp(void const *v1, void const *v2)
{
	struct map_entry const *e1 = v1, *e2 = v2;

	return strcmp(e1->key, e2->key);
}

void *
map_get(struct map *map, char *key)
{
	struct map_entry *entry, k = { .key = key };
	size_t sz;

	sz = sizeof(*map->entry);
	if ((entry = bsearch(&k, map->entry, map->len, sz, map_cmp)) == NULL)
		return NULL;
	return entry->value;
}

int
map_set(struct map *map, char *key, void *value)
{
	struct map_entry *insert, *e;
	size_t i, sz;
	void *v;

	debug("%s: key=%s len=%zd", __func__, key, map->len);

	for (i = 0; i < map->len; i++) {
		int cmp = strcmp(key, map->entry[i].key);
		debug("cmp(%s,%s)=%d", key, map->entry[i].key, cmp);

		if (cmp == 0) {
			map->entry[i].value = value;
			return 0;
		}
		if (cmp < 0)
			break;
	}

	sz = sizeof(*map->entry);
	if ((v = reallocarray(map->entry, map->len + 1, sz)) == NULL)
		return -1;
	map->entry = v;
	map->len++;

	insert = map->entry + i;
	e = map->entry + map->len - 1 - 1;
	for (; e >= insert; e--)
		e[1].key = e[0].key;

	if ((insert->key = strdup(key)) == NULL)
		return -1;
	insert->value = value;

	return 0;
}

int
map_del(struct map *map, char *key)
{
	size_t i;

	for (i = 0; i < map->len; i++) {
		int cmp = strcmp(key, map->entry[i].key);

		if (cmp == 0)
			break;
		if (cmp < 0)
			return -1;
	}
	if (i == map->len)
		return -1;

	map->len--;
	for (; i < map->len; i++)
		map->entry[i] = map->entry[i + 1];
	return 0;
}

void
map_init(struct map *map)
{
	memset(map, 0, sizeof(*map));
}

void
map_free_values(struct map *map)
{
	for (size_t i = 0; i < map->len; i++)
		free(map->entry[map->len - 1].value);
}

void
map_free(struct map *map)
{
	for (size_t i = 0; i < map->len; i++)
		free(map->entry[map->len - 1].key);
	free(map->entry);
}
