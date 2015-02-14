VERSION = 0.1

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

CFLAGS = -std=c99 -pedantic -Wall -Wextra -Wno-missing-field-initializers -Os
LDFLAGS = -s

CC = cc

# Change to 0755 to require sudo access
MODE = 4755
