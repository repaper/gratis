# Makefile

PREFIX ?= /usr

PLATFORM ?= ../RasberryPi
COG_VERSION ?= V1
EPD_IO ?= epd_io.h

FUSE_CFLAGS := $(shell pkg-config fuse --cflags)
FUSE_LDFLAGS := $(shell pkg-config fuse --libs)

CFLAGS += ${FUSE_CFLAGS}
CFLAGS += -Wall -Werror -std=gnu99
CFLAGS += -I${PLATFORM}
CFLAGS += -DEPD_IO='"${EPD_IO}"'

ifeq ($(strip ${COG_VERSION}),V1)
CFLAGS += -DEPD_COG_VERSION=1
EPD_H = epd_v1.h
EPD_O = epd_v1.o
else
ifeq ($(strip ${COG_VERSION}),V2)
CFLAGS += -DEPD_COG_VERSION=2
EPD_H = epd_v2.h
EPD_O = epd_v2.o
else
$(error unsupported COG_VERSION=${COG_VERSION})
endif
endif

LDFLAGS += ${FUSE_LDFLAGS}
LDFLAGS += -lrt

RM = rm -f


VPATH = .:${PLATFORM}

.PHONY: all
all: gpio_test epd_test epd_fuse

EPD_FUSE_CONF = ${PLATFORM}/epd-fuse.conf
EPD_FUSE_SH = ${PLATFORM}/epd-fuse.sh

.PHONY: install
install: epd_fuse
	install --group=root --mode=750 --owner=root epd_fuse "${PREFIX}/sbin"
	if [ -e "${EPD_FUSE_CONF}" ]  ; \
	then \
	  install --group=root --mode=644 --owner=root "${EPD_FUSE_CONF}" /etc/init ; \
	fi
	if [ -e "${EPD_FUSE_SH}" ] ; \
	then \
	  install --group=root --mode=755 --owner=root "${EPD_FUSE_SH}" /etc/init.d/epd-fuse ; \
	  [ -e /etc/default/epd-fuse ] || install --group=root --mode=644 --owner=root epd-fuse.default /etc/default/epd-fuse ; \
	  update-rc.d epd-fuse defaults ; \
	fi

.PHONY: remove
remove:
	[ -x /etc/init.d/epd-fuse ] && /etc/init.d/epd-fuse stop
	rm -f "${PREFIX}/sbin/epd_fuse"
	rm -f /etc/init/epd-fuse.conf
	rm -f /etc/init.d/epd-fuse
	update-rc.d epd-fuse remove
	[ -d /dev/epd ] && rmdir /dev/epd
	echo NOT Removing: /etc/default/epd-fuse

.PHONY: test-fuse
test-fuse:
	-sudo /etc/init.d/epd-fuse stop
	-sleep 1
	-sudo /etc/init.d/epd-fuse start
	-cat /dev/epd/panel
	-echo C > /dev/epd/command
	-sleep 1
	-./xbm2bin < venus_2_0.xbm > /dev/epd/display
	-echo U > /dev/epd/command
	-sleep 3
	-./xbm2bin < cat_2_0.xbm > /dev/epd/display
	-echo U > /dev/epd/command
	-sudo /etc/init.d/epd-fuse stop


# build the fuse driver
CLEAN_FILES += epd-fuse
epd_fuse: epd_fuse.o gpio.o spi.o ${EPD_O}
	${CC} ${CFLAGS} ${LDFLAGS} -o "$@" epd_fuse.o gpio.o spi.o ${EPD_O}


# build simple GPIO test program
CLEAN_FILES += gpio_test
gpio_test: gpio_test.o gpio.o
	${CC} ${CFLAGS} ${LDFLAGS} -o "$@" gpio_test.o gpio.o

# build EPD test program
CLEAN_FILES += epd_test
epd_test: epd_test.o gpio.o spi.o ${EPD_O}
	${CC} ${CFLAGS} ${LDFLAGS} -o "$@" epd_test.o gpio.o spi.o ${EPD_O}


# dependencies
gpio_test.o: gpio_test.h gpio.h
epd_test.o: gpio.h ${EPD_IO} spi.h ${EPD_H}
epd_fuse.o: gpio.h ${EPD_IO} spi.h ${EPD_H}

gpio.o: gpio.h
spi.o: spi.h
${EPD_O}: spi.h gpio.h ${EPD_H}


# clean up
.PHONY: clean
clean:
	${RM} ${CLEAN_FILES}
	${RM} *.o *.pyc *~
