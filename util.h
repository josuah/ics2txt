#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdarg.h>
#include <time.h>

/** logging **/
extern char *arg0;
void	 err(char const *fmt, ...);
void	 warn(char const *fmt, ...);
void	 debug(char const *fmt, ...);

/** strings **/
size_t	 strlcpy(char *, char const *, size_t);
char	*strsep(char **, char const *);
void	 strchomp(char *);
int	 strappend(char **, char const *);
size_t	 strlcat(char *, char const *, size_t);

/** memory **/
void	*reallocarray(void *, size_t, size_t);

/** time **/
time_t	 tztime(struct tm *, char const *);

#endif
