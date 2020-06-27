#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

/** src/log.c **/
int log_level;
char *log_arg0;
void vlogf(int level, char const *flag, char const *fmt, va_list va);
void die(char const *fmt, ...);
void warn(char const *fmt, ...);
void info(char const *fmt, ...);
void debug(char const *fmt, ...);

#endif
