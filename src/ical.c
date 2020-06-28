#include "ical.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> /* strcase* */

#include "util.h"

enum ical_err ical_errno;

int
ical_getline(char **line, char **ln, size_t *sz, FILE *fp)
{
	int c;
	void *v;

	if ((v = realloc(*line, 1)) == NULL)
		return -1;
	*line = v;
	(*line)[0] = '\0';

	do {
		if (getline(ln, sz, fp) <= 0)
			return ferror(fp) ? -1 : 0;
		strchomp(*ln);
		if (strappend(line, *ln) < 0)
			return -1;
		if ((c = fgetc(fp)) == EOF)
			return ferror(fp) ? -1 : 1;
	} while (c == ' ');

	ungetc(c, fp);
	assert(!ferror(fp));
	return 1;
}

char *
ical_strerror(int i)
{
	enum ical_err err = (i > 0) ? i : -i;

	switch (err) {
	case ICAL_ERR_OK:
		return "no error";
	case ICAL_ERR_SYSTEM:
		return "system error";
	case ICAL_ERR_END_MISMATCH:
		return "END: does not match its corresponding BEGIN:";
	case ICAL_ERR_MISSING_BEGIN:
		return "unexpected content line before any BEGIN:";
	case ICAL_ERR_MIN_NESTED:
		return "too many END: for the number of BEGIN:";
	case ICAL_ERR_MAX_NESTED:
		return "maximum nesting level reached";
	case ICAL_ERR_LENGTH:
		assert(!"used internally, should not happen");
	}
	assert(!"unknown error code");
	return "not a valid ical error code";
}

struct ical_value *
ical_new_value(char const *line)
{
	struct ical_value *new;
	size_t len;

	len = strlen(line);
	if ((new = calloc(1, sizeof *new + len + 1)) == NULL)
		return NULL;
	memcpy(new->buf, line, len + 1);
	return new;
}

void
ical_free_value(struct ical_value *value)
{
	debug("free value %p (%s:%s)", value, value->name, value->value);
	map_free(&value->param, free);
	free(value);
}

int
ical_parse_value(struct ical_value *value)
{
	char *column, *equal, *param, *cp;
	int e = errno;

	value->name = value->buf;

	if ((column = strchr(value->buf, ':')) == NULL)
		return -1;
	*column = '\0';
	value->value = column + 1;

	if ((cp = strchr(value->buf, ';')) != NULL)
		*cp++ = '\0';
	while ((param = strsep(&cp, ";")) != NULL) {
		if ((equal = strchr(param, '=')) == NULL)
			return -1;
		*equal = '\0';
		if (map_set(&value->param, param, equal + 1) < 0)
			return -1;
	}

	assert(errno == e);
	return 0;
}

struct ical_vnode *
ical_new_vnode(char const *name)
{
	struct ical_vnode *new;
	size_t sz;

	if ((new = calloc(1, sizeof *new)) == NULL)
		return NULL;
	sz = sizeof new->name;
	if (strlcpy(new->name, name, sz) >= sz) {
		errno = EMSGSIZE;
		goto err;
	}
	return new;
err:
	ical_free_vnode(new);
	return NULL;
}

static void
ical_free_value_void(void *v)
{
	ical_free_value(v);
}

static void
ical_free_vnode_void(void *v)
{
	ical_free_vnode(v);
}

void
ical_free_vnode(struct ical_vnode *node)
{
	if (node == NULL)
		return;
	debug("free vnode %p %s", node, node->name);
	map_free(&node->values, ical_free_value_void);
	map_free(&node->child, ical_free_vnode_void);
	ical_free_vnode(node->next);
	free(node);
}

int
ical_push_nested(struct ical_vcalendar *vcal, struct ical_vnode *new)
{
	struct ical_vnode **node;

	node = vcal->nested;
	for (int i = 0; *node != NULL; node++, i++) {
		if (i >= ICAL_NESTED_MAX)
			return -ICAL_ERR_MAX_NESTED;
	}
	node[0] = new;
	node[1] = NULL;
	return 0;
}

struct ical_vnode *
ical_pop_nested(struct ical_vcalendar *vcal)
{
	struct ical_vnode **node, **prev = vcal->nested, *old;

	for (prev = node = vcal->nested; *node != NULL; node++) {
		vcal->current = *prev;
		prev = node;
		old = *node;
	}
	*prev = NULL;
	if (vcal->nested[0] == NULL)
		vcal->current = NULL;
	return old;
}

int
ical_begin_vnode(struct ical_vcalendar *vcal, char const *name)
{
	struct ical_vnode *new;
	int e;

	if ((new = ical_new_vnode(name)) == NULL)
		return -ICAL_ERR_SYSTEM;
	if ((e = ical_push_nested(vcal, new)) < 0)
		goto err;
	if (vcal->root == NULL) {
		vcal->root = new;
	} else {
		new->next = map_get(&vcal->current->child, new->name);
		if (map_set(&vcal->current->child, new->name, new) < 0) {
			e = -ICAL_ERR_SYSTEM;
			goto err;
		}
	}
	vcal->current = new;
	return 0;
err:
	ical_free_vnode(new);
	return e;
}

int
ical_end_vnode(struct ical_vcalendar *vcal, char const *name)
{
	struct ical_vnode *old;

	if ((old = ical_pop_nested(vcal)) == NULL)
		return -ICAL_ERR_MIN_NESTED;
	if (strcasecmp(name, old->name) != 0)
		return -ICAL_ERR_END_MISMATCH;
	return 0;
}

int
ical_push_value(struct ical_vcalendar *vcal, struct ical_value *new)
{
	if (strcasecmp(new->name, "BEGIN") == 0) {
		int e = ical_begin_vnode(vcal, new->value);
		ical_free_value(new);
		return e;
	}
	if (strcasecmp(new->name, "END") == 0) {
		int e = ical_end_vnode(vcal, new->value);
		ical_free_value(new);
		return e;
	}

	if (vcal->current == NULL)
		return -ICAL_ERR_MISSING_BEGIN;

	debug("new %p %s:%s", new, new->name, new->value);
	new->next = map_get(&vcal->current->values, new->name);
	if (map_set(&vcal->current->values, new->name, new) < 0)
		return -ICAL_ERR_SYSTEM;

	return 0;
}

int
ical_read_vcalendar(struct ical_vcalendar *vcal, FILE *fp)
{
	char *line = NULL, *ln = NULL;
	size_t sz = 0;
	ssize_t r;
	int e;

	memset(vcal, 0, sizeof *vcal);

	while ((r = ical_getline(&line, &ln, &sz, fp)) > 0) {
		struct ical_value *new;

		if ((new = ical_new_value(line)) == NULL) {
			e = -ICAL_ERR_SYSTEM;
			goto err;
		}
		if ((e = ical_parse_value(new)) < 0)
			goto err;
		if ((e = ical_push_value(vcal, new)) < 0)
			goto err;
	}
	e = (r == 0) ? 0 : -ICAL_ERR_SYSTEM;
err:
	free(line);
	free(ln);
	return e;
}

void
ical_free_vcalendar(struct ical_vcalendar *vcal)
{
	debug("free vcalendar");
	ical_free_vnode(vcal->root);
}
