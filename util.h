#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define LEN(x) (sizeof (x) / sizeof *(x))

/** logging **/
extern char *arg0;
void	 err(int, char const *fmt, ...);
void	 warn(char const *fmt, ...);
void	 debug(char const *fmt, ...);

/** strings **/
size_t	 strlcpy(char *, char const *, size_t);
char	*strsep(char **, char const *);
void	 strchomp(char *);
char	*strappend(char **, char const *);
size_t	 strlcat(char *, char const *, size_t);
long long strtonum(const char *, long long, long long, const char **);
size_t	 strsplit(char *, char **, size_t, char const *);

/** memory **/
void	*reallocarray(void *, size_t, size_t);

/** time **/
time_t	 tztime(struct tm *, char const *);

#endif
