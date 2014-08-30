# Makefile

COG_VERSION ?= NOT-SET
EPD_IO ?= epd_io.h

SUPPORTED_VERSIONS = V1 V2

.PHONY: help
help:
	@echo
	@echo Please use the platform specific targets:
	@echo
	@echo COG version setting: add COG_VERSION=V1 or COG_VERSION=V2 to make
	@echo currently: COG_VERSION=${COG_VERSION}
	@echo
	@echo Custom I/O pin arrangement can be selected, add: EPD_IO=filename.h
	@echo '(the file will be taken from RaspberryPi or BeagleBone directory as appropriate)'
	@echo currently: EPD_IO=${EPD_IO}
	@echo
	@echo Raspberry Pi:
	@echo '   make rpi            = build all targets'
	@echo '   make rpi-install    = install fuse driver in PREFIX'
	@echo '   make rpi-T          = build only target T'
	@echo
	@echo BeagleBone
	@echo '   make bb             = build all targets'
	@echo '   make bb-install     = install fuse driver in PREFIX'
	@echo '   make bb-T           = build only target T'
	@echo
	@echo Where T is one of:
	@echo '    all install remove clean'
	@echo '    epd_test gpio_test epd_fuse'
	@echo
	@echo Notes:
	@echo 1. the default install PREFIX=/usr
	@echo 2. if not root must use sudo when installing
	@echo

.PHONY: all
all: help

.PHONY: cog-error
cog-error:
	@echo
	@echo ERROR: COG_VERSION was not set, please set to one of: ${SUPPORTED_VERSIONS}
	@echo

.PHONY: version-check
ifeq (,$(strip ${COG_VERSION}))
version-check: cog-error help
	@false
else
ifeq (NOT-SET,$(strip ${COG_VERSION}))
version-check: cog-error help
	@false
else
version-check:
	@true
endif
endif


.PHONY: rpi raspberrypi
rpi raspberrypi: version-check
	make PLATFORM=../RaspberryPi COG_VERSION="${COG_VERSION}" EPD_IO="${EPD_IO}" -C driver-common

rpi-%: version-check
	make PLATFORM=../RaspberryPi COG_VERSION="${COG_VERSION}" EPD_IO="${EPD_IO}" -C driver-common $*


.PHONY: bb beaglebone
bb beaglebone: version-check
	make PLATFORM=../BeagleBone COG_VERSION="${COG_VERSION}" EPD_IO="${EPD_IO}" -C driver-common

bb-%: version-check
	make PLATFORM=../BeagleBone COG_VERSION="${COG_VERSION}" EPD_IO="${EPD_IO}" -C driver-common $*
