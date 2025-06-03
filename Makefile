SHELL = /bin/bash

chip_arch = $(shell echo $(CHIP_ARCH) | tr A-Z a-z)

LIB_RLS_DIR :=
ifeq ($(DUAL_OS),y)
LIB_RLS_DIR = lib_$(SDK_VER)_dual
else
LIB_RLS_DIR = lib_$(SDK_VER)_single
endif
ifeq ($(wildcard $(LIB_RLS_DIR)),)
$(error $(LIB_RLS_DIR) not exist!)
else
$(shell rm -rf lib && cp -rlf $(LIB_RLS_DIR) lib)
endif

ifeq ($(PARAM_FILE), )
     PARAM_FILE:=Makefile.param
     include $(PARAM_FILE)
endif

ifeq ($(DESTDIR),)
    DESTDIR := $(shell pwd)/install
endif

$(info ** [ CHIP_ARCH ] ** = $(CHIP_ARCH))
$(info ** [ SDK_VER ] ** = $(SDK_VER))
$(info ** [ CROSS_COMPILE ] ** = $(CROSS_COMPILE))
$(info ** [ OS_TYPE ] ** = $(OS_TYPE))
$(info ** [ DESTDIR ] ** = $(DESTDIR))

.PHONY: all sample_app install uninstall clean

all: sample_app

ifeq ($(OS_TYPE), DUAL_OS)
SensorSupportList:
	@cd ../build/media/SensorSupportList/sensor_cfg && make
	@echo "SensorSupportList sensor is no need build in dual os"
else
SensorSupportList:
	@make -C ../build/media/SensorSupportList/ all
endif

sample_app: SensorSupportList
	@make -C sample_app/


install:
	@mkdir -p $(DESTDIR)/usr/bin
	@mkdir -p $(DESTDIR)/usr/lib/3rd
	# copy mw lib
	@cp -a lib/*.so*  $(DESTDIR)/usr/lib
	@cp -a lib/3rd/*.so*  $(DESTDIR)/usr/lib/3rd

uninstall:
	@rm $(DESTDIR) -rf

clean:
	@make -C sample_app/ clean
	@make -C ../build/media/SensorSupportList/ clean
