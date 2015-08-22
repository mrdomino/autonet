/* See LICENSE file for copyright and license details.  */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/queue.h>

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

#include "arg.h"

/*
 * Network profile preference.
 *
 * Maps discovered network nwids and/or bssids to known profile names.
 * Matches against nwid if non-NULL, and against bssid if other than BSSID_ANY.
 * If filename is NULL, nwid is the profile name.
 */
typedef struct {
	char*   nwid;
	uint8_t bssid[IEEE80211_ADDR_LEN];
	char*   filename;
} NetPref;

#define BSSID_ANY "\xff\xff\xff\xff\xff\xff"

#include "config.h"

#define HOSTNAME_IF "/etc/hostname." IFNAME

static char* const connect_cmd[] =
	{ "/bin/sh", "/etc/netstart", IFNAME, NULL };
static char* const connect_env[] = { NULL };
char* argv0;


static void
usage(void)
{
	fprintf(stderr,
	        "usage: %s [-v] [[-x excluded_profile] [-x ...] ...]\n"
	        "automatic wifi network chooser.\n"
	        "Creates an appropriate /etc/hostname."IFNAME" and "
	        "execs /etc/netstart "IFNAME".\n",
	        argv0);
	exit(1);
}

static const char*
NetPref_profile(const NetPref* np)
{
	return np->filename ? np->filename : np->nwid;
}

static bool
NetPref_match(const NetPref* np, const struct ieee80211_nodereq* nr)
{
	bool bssids_match, nwids_match;

	bssids_match =
		!memcmp(np->bssid, BSSID_ANY, IEEE80211_ADDR_LEN) ||
		!memcmp(np->bssid, nr->nr_bssid, IEEE80211_ADDR_LEN);
	nwids_match =
		np->nwid && strlen(np->nwid) == nr->nr_nwid_len &&
		!strncmp(np->nwid, (const char*)nr->nr_nwid, nr->nr_nwid_len);
	return bssids_match && nwids_match;
}

/*
 * Set up the hostname.if(5) symlink and exec netstart(8).
 *
 * Prints the profile name.
 * Links /etc/hostname.<ifname> -> /etc/hostname.d/<ifname>.<profile>.
 * Dies if /etc/hostname.<ifname> exists and is not a symlink.
 */
static void
NetPref_connect(const NetPref* np)
{
	struct stat sb;
	int r;
	char buf[256];
	const char *profile;

	profile = NetPref_profile(np);
	assert(profile);
	printf("network %s\n", profile);

	/* replace hostname.if(5) symlink for ifname */
	if (lstat(HOSTNAME_IF, &sb) < 0) {
		if (errno != ENOENT)
			err(1, "lstat");
	}
	else {
		if (!(S_ISLNK(sb.st_mode)))
			errx(1, HOSTNAME_IF " is not a symlink");
		if (unlink(HOSTNAME_IF) < 0)
			err(1, "unlink");
	}
	r = snprintf(buf, sizeof(buf), "hostname.d/"IFNAME".%s", profile);
	if (r < 0)
		err(1, "snprintf");
	assert((size_t)r < sizeof(buf));
	if (symlink(buf, HOSTNAME_IF) < 0)
		err(1, "symlink");
	/* exec netstart(8) */
	(void) execve(connect_cmd[0], connect_cmd, connect_env);
	err(1, "execve");
}

int
main(int argc, char* argv[])
{
	struct ieee80211_nodereq_all na;
	struct ieee80211_nodereq nr[512];
	struct ifreq ifr;
	int s, i;
	const NetPref *np;
	static SLIST_HEAD(np_excludes, np_exclude) excludes =
		SLIST_HEAD_INITIALIZER(excludes);
	struct np_exclude {
		char* npe_name;
		SLIST_ENTRY(np_exclude) npe_next;
	} *npe;

	ARGBEGIN {
	case 'v':
		fputs("autonet-"VERSION
		      ", (c) 2015 Steven Dee, see LICENSE for details\n",
		      stderr);
		exit(1);
	case 'x':
		npe = malloc(sizeof *npe);
		npe->npe_name = EARGF(usage());
		SLIST_INSERT_HEAD(&excludes, npe, npe_next);
		break;
	default:
		usage();
	} ARGEND;

	/* Bring the interface up and scan for networks */
	bzero(&ifr, sizeof(ifr));
	(void) strlcpy(ifr.ifr_name, IFNAME, sizeof(ifr.ifr_name));
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
	(void) strlcpy(na.na_ifname, IFNAME, sizeof(na.na_ifname));
	if (ioctl(s, SIOCG80211ALLNODES, &na) != 0)
		err(1, "SIOCG80211ALLNODES");
	close(s);

	/* Search for a match in order of preference */
	for (np = &networks[0]; NetPref_profile(np); np++) {
		SLIST_FOREACH(npe, &excludes, npe_next)
			if(!strcmp(npe->npe_name, NetPref_profile(np)))
				goto continue_outer;
		for (i = 0; i < na.na_nodes; i++)
			if (NetPref_match(np, &nr[i]))
				NetPref_connect(np);
continue_outer: ;
	}
	errx(2, "no known network found");
}
