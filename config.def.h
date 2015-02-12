#define IFNAME "iwn0"

static const NetPref networks[] = {
	/* nwid               bssid                                   filename*/
	{ "attwifi",          BSSID_NONE,                             NULL },
	{ "Free Public Wifi", BSSID_NONE,                             "fpw" },
	{ NULL,               { 0x69, 0xba, 0xad, 0x1d, 0xea, 0x70 }, "home" },
	{ 0 },
};
