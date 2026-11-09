/**
  Random Password Generator -- the engine. See pwgen.h for the rules.
  Author: Gabriel Castiglia - 2026-11-09
**/

#include <windows.h>
#include <bcrypt.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#include "pwgen.h"

#ifndef BCRYPT_USE_SYSTEM_PREFERRED_RNG
#define BCRYPT_USE_SYSTEM_PREFERRED_RNG 0x00000002
#endif

/* The four classes, built exactly the way password_gen.py builds them. */
typedef struct {
	char lower[27], upper[27], digits[11];
	const char* symbols;
	int nl, nu, nd, ns;
} Classes;

static void keepUnambiguous(const char* in, char* out)
{
	int n = 0;

	for (; *in; in++)
		if (!strchr(PW_AMBIGUOUS, *in)) out[n++] = *in;
	out[n] = 0;
}

static void buildClasses(int avoid, Classes* c)
{
	const char* lo = "abcdefghijklmnopqrstuvwxyz";
	const char* up = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const char* dg = "0123456789";

	if (avoid)
	{
		keepUnambiguous(lo, c->lower);
		keepUnambiguous(up, c->upper);
		keepUnambiguous(dg, c->digits);
	}
	else
	{
		strcpy(c->lower, lo);
		strcpy(c->upper, up);
		strcpy(c->digits, dg);
	}

	c->symbols = PW_SYMBOLS;
	c->nl = (int)strlen(c->lower);
	c->nu = (int)strlen(c->upper);
	c->nd = (int)strlen(c->digits);
	c->ns = (int)strlen(c->symbols);
}

/* A uniform integer in [0, n). Values from the top of the 32-bit range that would make
 * the low results more likely are thrown away and drawn again. */
static int randomBelow(unsigned n, unsigned* out)
{
	const uint64_t span = 0x100000000ULL;
	const uint64_t limit = span - (span % n);
	uint32_t x;

	for (;;)
	{
		if (BCryptGenRandom(NULL, (PUCHAR)&x, sizeof(x), BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0)
			return 0;
		if ((uint64_t)x < limit) { *out = (unsigned)(x % n); return 1; }
	}
}

static int pick(const char* set, int n, char* out)
{
	unsigned i;

	if (!randomBelow((unsigned)n, &i)) return 0;
	*out = set[i];
	return 1;
}

int pw_pool_size(int avoidAmbiguous)
{
	Classes c;

	buildClasses(avoidAmbiguous, &c);
	return c.nl + c.nu + c.nd + c.ns;
}

double pw_entropy_bits(int length, int avoidAmbiguous)
{
	return (double)length * log2((double)pw_pool_size(avoidAmbiguous));
}

int pw_generate(int length, int avoidAmbiguous, char* out)
{
	Classes c;
	char all[80];
	int nAll, i;

	if (length < PW_MIN_LEN) length = PW_MIN_LEN;
	if (length > PW_MAX_LEN) length = PW_MAX_LEN;

	buildClasses(avoidAmbiguous, &c);

	strcpy(all, c.lower);
	strcat(all, c.upper);
	strcat(all, c.digits);
	strcat(all, c.symbols);
	nAll = (int)strlen(all);

	/* one of each class first ... */
	if (!pick(c.lower, c.nl, &out[0]) || !pick(c.upper, c.nu, &out[1]) ||
	    !pick(c.digits, c.nd, &out[2]) || !pick(c.symbols, c.ns, &out[3]))
		goto failed;

	/* ... the rest from all of them ... */
	for (i = 4; i < length; i++)
		if (!pick(all, nAll, &out[i])) goto failed;

	/* ... and a Fisher-Yates shuffle, so the guaranteed four are not always in front. */
	for (i = length - 1; i > 0; i--)
	{
		unsigned j;
		char t;

		if (!randomBelow((unsigned)(i + 1), &j)) goto failed;
		t = out[i]; out[i] = out[j]; out[j] = t;
	}

	out[length] = 0;
	SecureZeroMemory(all, sizeof(all));
	return length;

failed:
	SecureZeroMemory(out, (size_t)length + 1);
	return 0;
}
