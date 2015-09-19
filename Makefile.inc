# See LICENSE file for copyright and license details.
VERSION=0.6
PREFIX=		/usr/local
BINDIR?=	${PREFIX}/bin
MANDIR?=	${PREFIX}/man/man
CFLAGS+=-DVERSION=\"${VERSION}\"
CDIAGFLAGS+=-std=c99 -pedantic -Wall -Wextra
