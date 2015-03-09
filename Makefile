# autonet - automatic network chooser
# See LICENSE file for copyright and license details.

include config.mk

all: options autonet netpref-line

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

autonet.o netpref-line.o: config.h config.mk

autonet: autonet.o
	@echo CC -o $@
	@${CC} -o $@ autonet.o ${LDFLAGS}

netpref-line: netpref-line.o
	@echo CC -o $@
	@${CC} -o $@ netpref-line.o ${LDFLAGS}

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
	@rm -f autonet autonet.o netpref-line netpref-line.o autonet-${VERSION}.tar.gz

install: all
	@echo installing executables to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f autonet ${DESTDIR}${PREFIX}/bin
	@chmod ${MODE} ${DESTDIR}${PREFIX}/bin/autonet
	@cp -f netpref-new ${DESTDIR}${PREFIX}/bin
	@chmod 0755 ${DESTDIR}${PREFIX}/bin/netpref-new
	@echo installing netpref-line to ${DESTDIR}${PREFIX}/libexec
	@mkdir -p ${DESTDIR}${PREFIX}/libexec
	@cp -f netpref-line ${DESTDIR}${PREFIX}/libexec
	@chmod 0755 ${DESTDIR}${PREFIX}/libexec/netpref-line
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@cp -f autonet.1 ${DESTDIR}${MANPREFIX}/man1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/autonet.1

uninstall:
	@echo removing executables from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/autonet
	@rm -f ${DESTDIR}${PREFIX}/bin/netpref-new
	@echo removing netpref-line from ${DESTDIR}${PREFIX}/libexec
	@rm -f ${DESTDIR}${PREFIX}/libexec/netpref-line

.PHONY: all options clean dist install uninstall
