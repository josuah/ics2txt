#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ical.h"
#include "log.h"
#include "util.h"

void
print_ruler(int level)
{
	for (int i = 0; i < level; i++)
		fprintf(stdout, ": ");
}

void
print_ical_tree_param(struct map_entry *entry, int level)
{
	if (entry == NULL)
		return;
	print_ruler(level);
	fprintf(stdout, "param %s=%s\n", entry->key, (char *)entry->value);
}

void
print_ical_tree_value(struct ical_value *value, int level)
{
	if (value == NULL)
		return;
	print_ruler(level);
	fprintf(stdout, "value %s:%s\n", value->name, value->value);
	for (size_t i = 0; i < value->param.len; i++)
		print_ical_tree_param(value->param.entry + i, level + 1);
	print_ical_tree_value(value->next, level);
}

void
print_ical_tree_vnode(struct ical_vnode *node, int level)
{
	if (node == NULL)
		return;
	print_ruler(level);
	fprintf(stdout, "node %p %s child=%lu next=%p\n",
	  (void *)node, node->name, node->child.len, (void *)node->next);
	for (size_t i = 0; i < node->values.len; i++)
		print_ical_tree_value(node->values.entry[i].value, level + 1);
	for (size_t i = 0; i < node->child.len; i++)
		print_ical_tree_vnode(node->child.entry[i].value, level + 1);
	print_ical_tree_vnode(node->next, level);
}

int
print_ical_tree(FILE *fp)
{
	struct ical_vcalendar vcal;
	int e;

	if ((e = ical_read_vcalendar(&vcal, fp)) < 0)
		die("reading ical file: %s", ical_strerror(e));

	print_ical_tree_vnode(vcal.root, 0);
	fprintf(stdout, ": end\n");
	fflush(stdout);

	ical_free_vcalendar(&vcal);
	return 0;
}

int
main(int argc, char **argv)
{
	log_arg0 = *argv++;

	if (*argv == NULL) {
		if (print_ical_tree(stdin) < 0)
			die("converting stdin");
	}

	for (; *argv != NULL; argv++, argc--) {
		FILE *fp;

		info("converting \"%s\"", *argv);
		if ((fp = fopen(*argv, "r")) == NULL)
			die("opening %s", *argv);
		if (print_ical_tree(fp) < 0)
			die("converting %s", *argv);
		fclose(fp);
	}

	return 0;
}
