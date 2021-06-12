#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdarg.h>

/* logging */
extern char *arg0;
void	 err(char const *fmt, ...);
void	 warn(char const *fmt, ...);
void	 debug(char const *fmt, ...);

/* strings */
size_t	 strlcpy(char *buf, char const *str, size_t sz);
char	*strsep(char **str_p, char const *sep);
void	 strchomp(char *line);
int	 strappend(char **base_p, char const *s);

/* memory */
void	*reallocarray(void *buf, size_t len, size_t sz);

#endif
