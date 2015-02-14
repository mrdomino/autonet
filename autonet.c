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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Network profile preference.
 *
 * Maps discovered network nwids and/or bssids to known profile names.
 * Matches against nwid if non-NULL, and against bssid if other than BSSID_NONE.
 * If filename is NULL, nwid is the profile name.
 */
typedef struct {
	/* TODO: numeric priority, connect to the network with highest signal
	 * strength at best priority.
	 */
	char*   nwid;
	uint8_t bssid[IEEE80211_ADDR_LEN];
	char*   filename;
} NetPref;

/* use the broadcast addr for BSSID_NONE; Xerox has OUI 00:00:00 */
#define BSSID_NONE { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }

#include "config.h"

static const char *ifname = IFNAME;
#define HOSTNAME_IF "/etc/hostname." IFNAME

/* XX The next two decls currently reduce binary size.
 * If commenting them doesn't yield a bigger binary, remove them.
 */
void errx(int, const char*, ...) __attribute__((noreturn));
void err(int, const char*, ...) __attribute__((noreturn));


static void
usage(const char* argv0)
{
	fprintf(stderr,
	        "usage: %s\n"
	        "automatic wifi network chooser.\n"
	        "Creates an appropriate /etc/hostname.%s and "
	        "execs /etc/netstart %s.\n",
	        argv0, ifname, ifname);
	exit(1);
}

static const char*
NetPref_profile(const NetPref* net_pref)
{
	if (net_pref->filename) {
		return net_pref->filename;
	}
	assert(net_pref->nwid);
	return net_pref->nwid;
}

static bool
NetPref_match(const NetPref* net_pref, const struct ieee80211_nodereq* nr)
{
	static const uint8_t bssid_none[IEEE80211_ADDR_LEN] = BSSID_NONE;
	bool bssids_match;

	if (0 != memcmp(net_pref->bssid, bssid_none, IEEE80211_ADDR_LEN))
		bssids_match = (0 == memcmp(net_pref->bssid, nr->nr_bssid,
		                            IEEE80211_ADDR_LEN));
	else bssids_match = true;
	if (!net_pref->nwid)
		return bssids_match;
	return bssids_match &&
	       strlen(net_pref->nwid) == nr->nr_nwid_len &&
	       (0 == strncmp(net_pref->nwid,
	                     (const char*)nr->nr_nwid,
	                     nr->nr_nwid_len));
}

/*
 * Set up the hostname.if(5) symlink and exec netstart(8).
 *
 * Prints the profile name.
 * Links /etc/hostname.<ifname> -> /etc/hostname.d/<ifname>.<profile>.
 * Dies if /etc/hostname.<ifname> exists and is not a symlink.
 */
static void
NetPref_connect(const NetPref* net_pref)
{
	struct stat sb;
	int r;
	char buf[256];

	printf("network %s\n", NetPref_profile(net_pref));
	/* replace hostname.if(5) symlink for ifname */
	if (lstat(HOSTNAME_IF, &sb) < 0) {
		if (errno != ENOENT)
			err(1, "lstat");
	}
	else {
		if (!(sb.st_mode & S_IFLNK))
			errx(1, HOSTNAME_IF " is not a symlink");
		if (unlink(HOSTNAME_IF) < 0)
			err(1, "unlink");
	}
	r = snprintf(buf, sizeof(buf), "hostname.d/%s.%s", ifname,
	             NetPref_profile(net_pref));
	if (r < 0)
		err(1, "snprintf");
	assert((size_t)r < sizeof(buf));
	if (symlink(buf, HOSTNAME_IF) < 0)
		err(1, "symlink");
	/* exec netstart(8) */
	(void) execl("/bin/sh", "/bin/sh", "/etc/netstart", ifname, NULL);
	err(1, "execl");
}

int
main(int argc, const char* argv[])
{
	struct ieee80211_nodereq_all na;
	struct ieee80211_nodereq nr[512];
	struct ifreq ifr;
	int s, i;
	const NetPref *net_pref;

	if (argc != 1)
		usage(argv[0]);

	/* Bring the interface up and scan for networks */
	bzero(&ifr, sizeof(ifr));
	(void) strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		err(1, "socket");
	if (ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr) < 0)
		err(1, "SIOCGIFFLAGS");
	if (!(ifr.ifr_flags & IFF_UP)) {
		ifr.ifr_flags |= IFF_UP;
		if (ioctl(s, SIOCSIFFLAGS, (caddr_t)&ifr) < 0)
			err(1, "SIOCSIFFLAGS");
	}
	/* TODO: exit successfully if network already active */
	if (ioctl(s, SIOCS80211SCAN, (caddr_t)&ifr) != 0)
		err(1, "SIOCS80211SCAN");

	/* Get all the networks we found */
	bzero(&na, sizeof(na));
	bzero(&nr, sizeof(nr));
	na.na_node = nr;
	na.na_size = sizeof(nr);
	(void) strlcpy(na.na_ifname, ifname, sizeof(na.na_ifname));
	if (ioctl(s, SIOCG80211ALLNODES, &na) != 0)
		err(1, "SIOCG80211ALLNODES");
	close(s);

	/* Search for a match in order of preference */
	for (net_pref = &networks[0];
	     net_pref->nwid || net_pref->filename; net_pref++) {
		for (i = 0; i < na.na_nodes; i++) {
			if (NetPref_match(net_pref, &nr[i])) {
				NetPref_connect(net_pref);
				/*NOTREACHED*/
			}
		}
	}
	errx(2, "no known network found");
	/*NOTREACHED*/
}
