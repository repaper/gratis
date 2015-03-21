# Makefile

PREFIX ?= /usr

PLATFORM ?= ../RasberryPi
PANEL_VERSION ?= v231_g2
EPD_IO ?= epd_io.h

FUSE_CFLAGS := $(shell pkg-config fuse --cflags)
FUSE_LDFLAGS := $(shell pkg-config fuse --libs)

# determine the epd.[ch] files path for the specific panel
EPD_DIR = $(notdir $(realpath $(strip ${PANEL_VERSION})))

ifeq (,$(strip ${EPD_DIR}))
$(error unsupported PANEL_VERSION=${PANEL_VERSION})
endif

# set up toolchaion options
CFLAGS += ${FUSE_CFLAGS}
CFLAGS += -Wall -Werror -std=gnu99
CFLAGS += -I${PLATFORM}
CFLAGS += -I${EPD_DIR}
CFLAGS += -I.
CFLAGS += -DEPD_IO='"${EPD_IO}"'

LDFLAGS += ${FUSE_LDFLAGS}
LDFLAGS += -lrt

RM = rm -f

UNAME_r := $(shell uname -r)
IS_ARCH := $(if $(findstring ARCH,${UNAME_r}),YES,NO)

VPATH = .:${PLATFORM}:${EPD_DIR}

.PHONY: all
all: gpio_test epd_test epd_fuse

EPD_FUSE_CONF = ${PLATFORM}/epd-fuse.conf
EPD_FUSE_SH = ${PLATFORM}/epd-fuse.sh

# abort if binary has not been built
.PHONY: check-compiled
check-compiled:
	[ -e epd_fuse ] || false

# install binary driver ans associated config/startup or service files
.PHONY: install
install: check-compiled
	install --group=root --mode=750 --owner=root epd_fuse "${PREFIX}/sbin"
	[ -e /etc/default/epd-fuse ] || install --group=root --mode=644 --owner=root epd-fuse.default /etc/default/epd-fuse
ifeq (YES,${IS_ARCH})
	install --group=root --mode=644 --owner=root epd-fuse.service "${PREFIX}/lib/systemd/system/epd-fuse.service"
	-systemctl daemon-reload
	-systemctl enable epd-fuse
else # otherwise Debian
	if [ -e "${EPD_FUSE_CONF}" ] ; \
	then \
	  install --group=root --mode=644 --owner=root "${EPD_FUSE_CONF}" /etc/init ; \
	fi
	if [ -e "${EPD_FUSE_SH}" ] ; \
	then \
	  install --group=root --mode=755 --owner=root "${EPD_FUSE_SH}" /etc/init.d/epd-fuse ; \
	  update-rc.d epd-fuse defaults ; \
	fi
endif

.PHONY: remove
remove:
	[ -x /etc/init.d/epd-fuse ] && /etc/init.d/epd-fuse stop
	rm -f "${PREFIX}/sbin/epd_fuse"
ifeq (YES,${IS_ARCH})
	-systemctl stop epd-fuse
	-systemctl disable epd-fuse
	-systemctl daemon-reload
	rm -f "${PREFIX}/lib/systemd/system/epd-fuse.service"
else # otherwise Debian
	rm -f /etc/init/epd-fuse.conf
	rm -f /etc/init.d/epd-fuse
	update-rc.d epd-fuse remove
endif
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


# low-level driver
DRIVER_OBJECTS = gpio.o spi.o epd.o
GPIO_OBJECTS = gpio_test.o gpio.o
FUSE_OBJECTS = epd_fuse.o ${DRIVER_OBJECTS}
TEST_OBJECTS = epd_test.o ${DRIVER_OBJECTS}

# build the fuse driver
CLEAN_FILES += epd-fuse
epd_fuse: ${FUSE_OBJECTS}
	${CC} ${CFLAGS} ${LDFLAGS} -o "$@" ${FUSE_OBJECTS}


# build simple GPIO test program
CLEAN_FILES += gpio_test
gpio_test: ${GPIO_OBJECTS}
	${CC} ${CFLAGS} ${LDFLAGS} -o "$@" ${GPIO_OBJECTS}

# build EPD test program
CLEAN_FILES += epd_test
epd_test: ${TEST_OBJECTS}
	${CC} ${CFLAGS} ${LDFLAGS} -o "$@" ${TEST_OBJECTS}


# dependencies
gpio_test.o: gpio_test.h gpio.h
epd_test.o: gpio.h ${EPD_IO} spi.h epd.h
epd_fuse.o: gpio.h ${EPD_IO} spi.h epd.h

gpio.o: gpio.h
spi.o: spi.h
epd.o: spi.h gpio.h epd.h


# clean up
.PHONY: clean
clean:
	${RM} ${CLEAN_FILES}
	${RM} *.o *.pyc *~
