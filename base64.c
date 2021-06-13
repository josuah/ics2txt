#include "base64.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>

static char encode_map[64] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void
base64_encode(char const *s, size_t slen, char *d, size_t *dlen)
{
	char const *sbeg = s, *send = s + slen, *dbeg = d;
	unsigned char x;

	while (s < send) {
		switch ((s - sbeg) % 3) {
		case 0: /* AAAAAABB bbbbcccc ccdddddd */
			assert((size_t)(d - dbeg) + 1 < *dlen);
			*d++ = encode_map[*s >> 2];
			x = *s << 4 & 0x3f;
			break;
		case 1: /* aaaaaabb BBBBCCCC ccdddddd */
			assert((size_t)(d - dbeg) + 1 < *dlen);
			*d++ = encode_map[x | (*s >> 4)];
			x = (*s << 2) & 0x3f;
			break;
		case 2: /* aaaaaabb bbbbcccc CCDDDDDD */
			assert((size_t)(d - dbeg) + 2 < *dlen);
			*d++ = encode_map[x | (*s >> 6)];
			*d++ = encode_map[*s & 0x3f];
			break;
		}
		s++;
	}

	/* flush extra content in 'x' */
	assert((size_t)(d - dbeg) + 1 < *dlen);
	if ((s - sbeg) % 3 != 2)
		*d++ = encode_map[x];

	/* pad the end with '=' */
	while ((d - dbeg) % 4 != 0) {
		assert((size_t)(d - dbeg) + 1 < *dlen);
		*d++ = '=';
	}

	*dlen = d - dbeg;
}

static int8_t decode_map[256] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1,  0, -1, -1,
	-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

int
base64_decode(char const *s, size_t *slen, char *d, size_t *dlen)
{
	char const *sbeg = s, *send = sbeg + *slen, *dbeg = d;

	for (; s + 3 < send; s += 4) {
		int8_t x0 = decode_map[(unsigned)s[0]];
		int8_t x1 = decode_map[(unsigned)s[1]];
		int8_t x2 = decode_map[(unsigned)s[2]];
		int8_t x3 = decode_map[(unsigned)s[3]];
		uint32_t x = (x0 << 18) | (x1 << 12) | (x2 << 6) | (x3 << 0);

		assert((size_t)(d - dbeg) + 3 < *dlen);
		*d++ = x >> 16;
		*d++ = x >> 8 & 0xff;
		*d++ = x & 0xff;

		/* only "xxxx" or "xxx=" or "xx==" allowed  */
		if (s[0] == '=' || s[1] == '=' || (s[2] == '=' && s[3] != '='))
			return -2;
		if (s[2] == '=')
			d--;
		if (s[3] == '=') {
			d--;
			break;
		}

		if (x0 < 0 || x1 < 0 || x2 < 0 || x3 < 0)
			return -1;
	}

	*slen = s - sbeg;
	*dlen = d - dbeg;
	return 0;
}
