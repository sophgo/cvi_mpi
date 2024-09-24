SHELL = /bin/bash
#
#
PARAM_FILE	:= $(CURDIR)/mpi_param.mk
include $(PARAM_FILE)
#
.PHONY: clean all module sample install uninstall package 3rdparty self_test clean_all

all: prepare 3rdparty module sample self_test

module: prepare 3rdparty
	@make -C modules/

prepare:
	@if [ -f "mpi_prepare.mk" ]; then \
		make -f mpi_prepare.mk prepare; \
	fi

3rdparty:
	@if [ -d "3rdparty/" ]; then \
		make -C 3rdparty/; \
	fi

sample: module
	@if [ -d "sample/" ]; then \
		make -C sample/; \
	fi

self_test: module sample
	@if [ -d "self_test/" ]; then \
		make -C self_test/; \
	fi
#
clean:
	@make -C modules/ clean
	@make -C 3rdparty/ clean
	@make -C sample/ clean
	@if [ -d "self_test/" ]; then \
		make -C self_test/ clean; \
	fi
	@if [ -f "mpi_prepare.mk" ]; then \
		make -f mpi_prepare.mk clean; \
	fi

clean_all:
	@make -C modules/ clean_all
