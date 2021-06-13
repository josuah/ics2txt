#ifndef BASE64_H
#define BASE64_H

#include <stddef.h>

void	base64_encode(char const *, size_t, char *, size_t *);

/*
 * It is possible to use the same variables for both source and
 * destination. Then the base64 will overwrite the source buffer
 * with the destination data.
 *
 * If the same pointer is passed as both source and destination
 * size, the source size will be inaccurate but the destination
 * will be correct.
 */
int	base64_decode(char const *, size_t *, char *, size_t *);

#endif
