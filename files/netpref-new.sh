#!/bin/sh
# See LICENSE file for copyright and license details.

PREFIX=$(dirname $(dirname $0))
argv0=$(basename $0)

function usage {
	echo "Usage: $argv0 [profile-name]" 1>&2
	exit 1
}

function netpref_new {
	local filename="$1"
	local nwid=$(grep -e '^nwid ' $filename | cut -d' ' -f2-)
	local bssid=$(grep -e '^bssid ' $filename | cut -d' ' -f2-)
	local profile=$(grep -e '^# profile ' $filename | cut -d' ' -f3-)
	$PREFIX/libexec/netpref-line "$nwid" "${bssid:--}" "$profile" > "$CONFIG_H_LINE" &&
		sed -e '/^\/\* netpref-new added \*\/$/{
			r '"$CONFIG_H_LINE"'
		}' "$CONFIG_H" > "$CONFIG_H_NEW" &&
		{ echo "new config.h:"
		  cat "$CONFIG_H_NEW"
		  echo accept?
		  read &&
		  { cp -f "$CONFIG_H_NEW" "$CONFIG_H" &&
		    cp -i "$filename" "/etc/hostname.d/$IFNAME.${profile:-$nwid}"
		  }
		}
}


[ 1 -ge $# ] || usage
[ "x$1" = "x--help" ] && usage

IFNAME=iwn0
PROFILE=$1
CONFIG_H=/usr/ports/mystuff/net/autonet/files/config.h

HOSTNAME_IF=$(mktemp -t "iwn0.$PROFILE.XXXXXXXX")
CONFIG_H_LINE=$(mktemp -t)
CONFIG_H_NEW=$(mktemp -t config.h.XXXXXXXX)
trap "rm -f '$HOSTNAME_IF' '$CONFIG_H_LINE' '$CONFIG_H_NEW'" EXIT
if [ -n "$PROFILE" ]
then
	echo "# profile $PROFILE" > "$HOSTNAME_IF"
fi
${EDITOR:-vim} "$HOSTNAME_IF" &&
	netpref_new "$HOSTNAME_IF" "$PROFILE"
