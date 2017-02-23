# $OpenBSD: Makefile.template,v 1.76 2017/02/25 16:04:20 danj Exp $

COMMENT =	simple wifi network chooser

VERSION =	0.7
PKGNAME =	autonet-${VERSION}
CATEGORIES =	net
DISTFILES =
WANTLIB =		c
NO_TEST =	Yes
CFLAGS +=	-DVERSION=\"${VERSION}\"

# BSD
PERMIT_PACKAGE_CDROM =	Yes

post-extract:
	@lndir ${FILESDIR} ${WRKDIR}

MAINTAINER =		Steven Dee <i@wholezero.org>

do-install:
	${INSTALL_PROGRAM} ${WRKSRC}/autonet ${PREFIX}/bin/autonet
	${INSTALL_PROGRAM} ${WRKSRC}/netpref-new ${PREFIX}/bin/netpref-new
	${INSTALL_PROGRAM} ${WRKSRC}/netpref-line ${PREFIX}/libexec/netpref-line
	${INSTALL_DATA} ${WRKSRC}/autonet.1 ${PREFIX}/man/man1
	${INSTALL_DATA_DIR} ${PREFIX}/share/doc/autonet
	${INSTALL_DATA} ${WRKSRC}/rc-autonet.patch ${PREFIX}/share/doc/autonet

.include <bsd.port.mk>
