# Makefile

.PHONY: help
help:
	@echo
	@echo Please use the platform specific targets:
	@echo
	@echo Raspberry Pi:
	@echo '   make rpi            = build all targets'
	@echo '   make rpi-install    = install fuse driver in PREFIX'
	@echo '   make rpi-T          = build only target T'
	@echo
	@echo Beagle Bone
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


.PHONY: rpi raspberrypi
rpi raspberrypi:
	make PLATFORM=../RaspberryPi -C driver-common

rpi-%:
	make PLATFORM=../RaspberryPi -C driver-common $*


.PHONY: bb beaglebone
bb beaglebone:
	make PLATFORM=../BeagleBone -C driver-common

bb-%:
	make PLATFORM=../BeagleBone -C driver-common $*
