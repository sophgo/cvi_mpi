ifeq ($(CHIP_ARCH), $(filter $(CHIP_ARCH), CV181X))
	SUBDIRS := fastboot cipher vio datafifo vdecvo ipcm ive sensor_test ipcmsg vdec sample_ir_auto audio sample_panel venc region
else ifeq ($(CHIP_ARCH), $(filter $(CHIP_ARCH), CV180X))
	SUBDIRS := fastboot cipher datafifo ipcm sensor_test ipcmsg vdec sample_ir_auto audio venc region
endif

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
