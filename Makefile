SHELL = /bin/bash
#
#
PARAM_FILE	:= $(CURDIR)/mpi_param.mk
include $(PARAM_FILE)
#
.PHONY: clean all module sample install uninstall package 3rdparty self_test clean_all

all: prepare 3rdparty module sample self_test

module: prepare 3rdparty
	@rm -f $(MW_LIB)/Target:*.txt
	@$(CC) -v 2> $(MW_LIB)/Target:"$(TARGET_MACHINE)".txt
	@make -C modules/

prepare:
	@if [ -f "mpi_prepare.mk" ]; then \
		make -f mpi_prepare.mk prepare; \
		sed -i '19i#ifndef __CV181X__\n\t#define __CV181X__\n#endif\n' include/cvi_defines.h; \
	fi
	@if [ "$(CHIP_ARCH)" = "CV181X" ]; then \
		sed -i '19,21c\#ifndef __CV181X__\n\t#define __CV181X__\n#endif' include/cvi_defines.h; \
	elif [ "$(CHIP_ARCH)" = "CV180X" ]; then \
		sed -i '19,21c\#ifndef __CV180X__\n\t#define __CV180X__\n#endif' include/cvi_defines.h; \
	else \
		echo "Unknown chip architecture $(CHIP_ARCH)"; \
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
	@rm -f $(MW_LIB)/Target:*.txt

clean_all:
	@make -C modules/ clean_all

install:
	@if find $(MW_LIB) -maxdepth 1 -name "Target:*.txt" | read; then \
		_FILE_NAME=$$(find $(MW_LIB) -maxdepth 1 -name "Target:*.txt" | xargs echo); \
		_TARGER_MACHINE=$$(echo "$${_FILE_NAME}" | sed 's/.*Target://; s/.txt$$//'); \
		echo "MPI install dir is: $(MPI_INSTALL_DIR)/$$_TARGER_MACHINE/cvi_mpi"; \
		mkdir -p $(MPI_INSTALL_DIR)/$$_TARGER_MACHINE/cvi_mpi; \
		cp -rf $(MW_PATH)/include $(MPI_INSTALL_DIR)/$$_TARGER_MACHINE/cvi_mpi; \
		cp -rf $(MW_PATH)/lib $(MPI_INSTALL_DIR)/$$_TARGER_MACHINE/cvi_mpi; \
	else \
		echo "file not exists"; \
		exit 1; \
	fi
