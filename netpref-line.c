#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ADDRILEN (sizeof "xx:xx:xx:xx:xx:xx" - 1)
#define ADDROLEN (6 * (sizeof "\\x00" - 1))

#define bssid_quote(I, O) \
	(assert(sizeof(O) > ADDROLEN + 2), \
	 _bssid_quote(I, O))

/* Doesn't null-terminate out. */
static int
_bssid_escape(const char* in, char* out)
{
	char *c = out;
	size_t i;

	if (strlen(in) != ADDRILEN)
		return -1;
	for (i = 0; i < ADDRILEN; i++) {
		if ((i % 3) == 0) {
			*c++ = '\\';
			*c++ = 'x';
		}
		if ((i % 3) != 2) {
			if (!isxdigit(in[i]))
				return -1;
			*c++ = in[i];
		}
		else continue;
	}
	return c - out;
}

static int
_bssid_quote(const char* in, char* out)
{
	int r;

	if (*in == '-') {
		r = snprintf(out, ADDROLEN + 3, "%s", "BSSID_ANY");
		assert(r >= 0);
		assert((size_t)r < ADDROLEN + 3);
		return r;
	}
	r = _bssid_escape(in, out + 1);
	if (r == -1)
		return -1;
	*out = '"';
	*(out + r) = '"';
	*(out + r + 1) = '\0';
	return r + 3;

}

static size_t
c_escape(const char* in, char* out, size_t len)
{
	size_t r = 1, s;
	const char *c;

	for (c = in; *c != '\0'; c++) {
		if (iscntrl(*c) || !isascii(*c))
			r += 4;
		else {
			if (*c == '\"' || *c == '\\')
				r += 2;
			else r++;
		}
	}
	if (r > len)
		return r;
	for (c = in; *c != '\0'; c++) {
		if (iscntrl(*c) || !isascii(*c)) {
			s = snprintf(out, len, "\\x%02hhx", *c);
			assert(s == 4);
		}
		else {
			if (*c == '\"' || *c == '\\') {
				s = snprintf(out, len, "\\%c", *c);
				assert(s == 2);
			}
			else {
				*out = *c;
				s = 1;
			}
		}
		assert(s < len);
		out += s;
		len -= s;
	}
	assert(len > 0);
	*out = '\0';
	return r;
}

static size_t
profile_name(const char* in, char* out, size_t len)
{
	static const char *nul = "NULL";

	if (*in == '\0')
		return strlcpy(out, nul, len);
	(void) strlcpy(out, "\"", len);
	(void) strlcat(out, in, len);
	return strlcat(out, "\"", len);
}

int
main(int argc, char* argv[])
{
	char qbssid[ADDROLEN + 3];
	char nwid[1024];
	char profile[64];
	int r;

	if (argc != 4)
		errx(1, "argc");
	if ((r = bssid_quote(argv[2], qbssid)) == -1)
		errx(1, "bssid_quote");
	if (sizeof nwid < c_escape(argv[1], nwid, sizeof nwid))
		errx(1, "nwid");
	if (sizeof profile < profile_name(argv[3], profile, sizeof profile))
		errx(1, "profile");
	printf("\t{ \"%s\", %s, %s },\n", nwid, qbssid, profile);
	exit(0);
}
