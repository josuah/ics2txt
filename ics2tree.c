#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ical.h"
#include "log.h"
#include "util.h"

void
print_ical_tree_param(struct map_entry *entry, int level)
{
	if (entry == NULL)
		return;
	for (int i = 0; i < level; i++)
		printf(": ");
	fprintf(stdout, "param %s=%s\n", entry->key, (char *)entry->value);
}

void
print_ical_tree_value(struct ical_value *value, int level)
{
	if (value == NULL)
		return;
	for (int i = 0; i < level; i++)
		printf(": ");
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
	for (int i = 0; i < level; i++)
		printf(": ");
	fprintf(stdout, "node %p %s child=%p next=%p\n", node, node->name, node->child, node->next);
	for (size_t i = 0; i < node->values.len; i++)
		print_ical_tree_value(node->values.entry[i].value, level + 1);
	print_ical_tree_vnode(node->child, level + 1);
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
	fprintf(stdout, ".\n");
	fflush(stdout);

	ical_free_vcalendar(&vcal);
	return 0;
}

void
print_header(void)
{
	char *fields[] = { "", NULL };

	printf("%s\t%s", "beg", "end");

	for (char **f = fields; *f != NULL; f++) {
		fprintf(stdout, "\t%s", *f);
	}
	fprintf(stdout, "\n");
}

int
main(int argc, char **argv)
{
	print_header();

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
