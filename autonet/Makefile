# See LICENSE file for copyright and license details.

PROG=	autonet
CFLAGS+=-DVERSION=\"${VERSION}\"
CDIAGFLAGS+=-std=c99 -pedantic -Wall -Wextra -Wno-missing-field-initializers

autonet.o: ${.CURDIR}/../config.h

${.CURDIR}/../config.h: ${.CURDIR}/../config.def.h
	cp ${.CURDIR}/../config.def.h ${.CURDIR}/../config.h

.include "../Makefile.inc"
.include <bsd.prog.mk>
