VERSION = 0.2

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

CFLAGS = -std=c99 -pedantic -Wall -Wextra -DVERSION=\"${VERSION}\" \
         -Wno-missing-field-initializers -Os
LDFLAGS = -s

CC = cc

# Change to 4755 to not require sudo access
MODE = 0755
