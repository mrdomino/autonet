/* See LICENSE file for copyright and license details.  */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <net/if.h>
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_ioctl.h>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * At least one of {nwid, bssid} must be non-NULL.
 *
 * If filename is NULL, nwid must not be NULL, and is used as the filename.
 */
typedef struct {
	char*   nwid;
	uint8_t bssid[IEEE80211_ADDR_LEN];
	char*   filename;
} NetPref;

#define BSSID_NONE { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
static const uint8_t bssid_none[IEEE80211_ADDR_LEN] = BSSID_NONE;

#include "config.h"

static const char *ifname = IFNAME;
#define HOSTNAME_IF "/etc/hostname." IFNAME


static const char*
NetPref_filename(const NetPref* net_pref)
{
	if (net_pref->filename) {
		return net_pref->filename;
	}
	assert(net_pref->nwid);
	return net_pref->nwid;
}

static void
NetPref_print_filename(const NetPref* net_pref, char* buf, size_t len)
{
	size_t r;

	(void) strlcpy(buf, "hostname.d/", len);
	(void) strlcat(buf, ifname, len);
	(void) strlcat(buf, ".", len);
	r = strlcat(buf, NetPref_filename(net_pref), len);
	assert(r < len);
}

static int
NetPref_match(const NetPref* net_pref, const struct ieee80211_nodereq* nr)
{
	int match;

	if (0 != memcmp(net_pref->bssid, bssid_none, IEEE80211_ADDR_LEN))
		match = (0 == memcmp(net_pref->bssid, nr->nr_bssid,
		                     IEEE80211_ADDR_LEN));
	else match = 1;
	if (net_pref->nwid)
		match = match && (0 == strncmp(net_pref->nwid,
		                               (const char*)nr->nr_nwid,
		                               nr->nr_nwid_len));
	return match;
}

#ifndef NDEBUG
#define dprintf(...) fprintf(stderr, __VA_ARGS__)
#else
#define dprintf(...) 0
#endif

int main(void)
{
	struct stat sb;
	struct ieee80211_nodereq_all na;
	struct ieee80211_nodereq nr[512];
	struct ifreq ifr;
	int s, i, flags, found = 0;
	const NetPref *net_pref;
	char filename[256];

	if (lstat(HOSTNAME_IF, &sb) < 0) {
		if (errno != ENOENT)
			err(1, "lstat");
	}
	else {
		if (!(sb.st_mode & S_IFLNK)) {
			errx(1, HOSTNAME_IF " is not a symlink");
		}
		if (unlink(HOSTNAME_IF) < 0) {
			err(1, "unlink");
		}
	}

	bzero(&ifr, sizeof(ifr));
	ifr.ifr_addr.sa_family = AF_INET;
	(void) strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		err(1, "socket");
	if (ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr) < 0)
		err(1, "SIOCGIFFLAGS");
	flags = ifr.ifr_flags;
	if (!(flags & IFF_UP)) {
		dprintf("bringing up if\n");
		flags |= IFF_UP;
		ifr.ifr_flags = flags;
		if (ioctl(s, SIOCSIFFLAGS, (caddr_t)&ifr) < 0)
			err(1, "SIOCSIFFLAGS");
	}
	if (ioctl(s, SIOCS80211SCAN, (caddr_t)&ifr) != 0)
		err(1, "SIOCS80211SCAN");

	bzero(&na, sizeof(na));
	bzero(&nr, sizeof(nr));
	na.na_node = nr;
	na.na_size = sizeof(nr);
	(void) strlcpy(na.na_ifname, ifname, sizeof(na.na_ifname));

	if (ioctl(s, SIOCG80211ALLNODES, &na) != 0)
		err(1, "SIOCG80211ALLNODES");
	close(s);

	for (i = 0; i < na.na_nodes; i++) {
		dprintf("trying %.*s\n", nr[i].nr_nwid_len, nr[i].nr_nwid);
		net_pref = &networks[0];
		while (net_pref->nwid || net_pref->filename) {
			if (NetPref_match(net_pref, &nr[i])) {
				NetPref_print_filename(net_pref, filename,
				                       sizeof(filename));
				found = 1;
				break;
			}
			net_pref++;
		}
		if (found)
			break;
	}

	if (found) {
		dprintf("found profile:%s\n", filename);
		if (symlink(filename, HOSTNAME_IF) < 0) {
			err(1, "symlink");
		}
	}
	else {
		errx(1, "no known network found");
	}

	execl("/bin/sh", "/bin/sh",
	      "/etc/netstart", ifname, NULL);
	err(1, "execl");
}
