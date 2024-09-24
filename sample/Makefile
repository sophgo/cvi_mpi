SUBDIRS := $(notdir $(basename $(shell find . -maxdepth 1 -type d)))
ifeq ($(CHIP_ARCH), $(filter $(CHIP_ARCH), CV181X))
	exclude_dirs = common
else ifeq ($(CHIP_ARCH), $(filter $(CHIP_ARCH), CV180X))
	exclude_dirs = common vdecvo mipi_tx sample_panel overlay vio ive
endif
SUBDIRS := $(filter-out $(exclude_dirs), $(SUBDIRS))

all:
	@echo "##############################"
	@echo "#                            #"
	@echo "#   Compiling 'samples'...   #"
	@echo "#                            #"
	@echo "##############################"
	@echo $(SUBDIRS) $(MAKECMDGOALS)

	@$(foreach t,$(SUBDIRS), \
		cd $(t) && $(MAKE) all || exit 1 && cd -; )

clean:
	@$(foreach t,$(SUBDIRS), \
		cd $(t) && $(MAKE) clean || exit 1 && cd -; )
