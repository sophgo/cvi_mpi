ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../mpi_param.mk
     include $(PARAM_FILE)
endif

.PHONY: all clean clean_all

SUB_DIRS := ipcm cvi_link msg misc sys vi vpss vo bin rgn gdc venc vdec ive mipi_tx audio isp

all:
	@echo "########################################################################################################"
	@echo "#"
	@echo "#  Compiling 'module libs' Configs as below ..."
	@echo "#    "CROSS_COMPILE=$(CROSS_COMPILE)
	@echo "#"
	@echo "########################################################################################################"
	@$(foreach dir,$(SUB_DIRS), \
		if test -d $(dir) ; then \
			cd $(dir);$(MAKE) all;cd -; \
		else \
			exit 1; \
		fi;)

clean:
	@$(foreach dir,$(SUB_DIRS), \
		if test -d $(dir) ; then \
			cd $(dir);$(MAKE) clean;cd -; \
		else \
			exit 1; \
		fi;)

clean_all:
	@$(foreach dir,$(SUB_DIRS), \
		if test -d $(dir) ; then \
			cd $(dir);$(MAKE) clean_all;cd -; \
		else \
			exit 1; \
		fi;)
