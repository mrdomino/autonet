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

clean:
	@echo cleaning
	@rm -f autonet autonet.o

install: all
	@echo installing executable to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f autonet ${DESTDIR}${PREFIX}/bin
	@chmod 04755 ${DESTDIR}${PREFIX}/bin/autonet

uninstall:
	@echo removing executable from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/autonet

.PHONY: all options clean install uninstall
