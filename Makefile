# autonet - automatic network chooser
# See LICENSE file for copyright and license details.

include config.mk

all: options autonet

options:
	@echo autonet build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC -c $<
	@${CC} -c $< ${CFLAGS}

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

autonet.o: config.h config.mk

autonet: autonet.o
	@echo CC -o $@
	@${CC} -o $@ autonet.o ${LDFLAGS}

dist: clean
	@echo creating dist tarball
	@mkdir -p autonet-${VERSION}
	@cp -R LICENSE Makefile config.mk config.def.h README \
		autonet.c autonet.1 examples autonet-${VERSION}
	@tar -cf autonet-${VERSION}.tar autonet-${VERSION}
	@gzip autonet-${VERSION}.tar
	@rm -rf autonet-${VERSION}

clean:
	@echo cleaning
	@rm -f autonet autonet.o autonet-${VERSION}.tar.gz

install: all
	@echo installing executable to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f autonet ${DESTDIR}${PREFIX}/bin
	@chmod ${MODE} ${DESTDIR}${PREFIX}/bin/autonet
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@cp -f autonet.1 ${DESTDIR}${MANPREFIX}/man1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/autonet.1

uninstall:
	@echo removing executable from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/autonet

.PHONY: all options clean dist install uninstall
