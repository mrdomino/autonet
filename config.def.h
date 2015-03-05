#define IFNAME "iwn0"

static const NetPref networks[] = {
	/* nwid               bssid                        filename */
	{ "attwifi",          BSSID_ANY,                  NULL      },
	{ "Free Public Wifi", BSSID_ANY,                  "fpw"     },
	{ NULL,               "\x69\xba\xad\xbe\xef\x70",  "home"   },
/* netpref-new added */
/* end */
	{ 0 },
};
