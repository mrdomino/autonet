# See LICENSE file for copyright and license details.

PROG=		autonet
LDSTATIC=	${STATIC}
CDIAGFLAGS+=-Wno-missing-field-initializers

autonet.o: ${.CURDIR}/../config.h

${.CURDIR}/../config.h: config.def.h
	cp ${.ALLSRC} ${.TARGET}

.include "../Makefile.inc"
.include <bsd.prog.mk>
