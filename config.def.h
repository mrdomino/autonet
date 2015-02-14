#define IFNAME "iwn0"

static const NetPref networks[] = {
	/* nwid               bssid                        filename */
	{ "attwifi",          BSSID_NONE,                  NULL     },
	{ "Free Public Wifi", BSSID_NONE,                  "fpw"    },
	{ NULL,               "\x69\xba\xad\xbe\xef\x70",  "home"   },
	{ 0 },
};
