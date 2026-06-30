ifeq ($(filter %/aarch64-linux-gnu-,$(CROSS_COMPILE)),$(CROSS_COMPILE))
    $(info Skipping build for aarch64-linux-gnu- compiler)
    all clean install: ;
else ifeq ($(filter %/arm-linux-gnueabihf-,$(CROSS_COMPILE)),$(CROSS_COMPILE))
    $(info Skipping build for arm-linux-gnueabihf- compiler)
    all clean install: ;
else
TOPTARGETS := all clean install

SUBDIRS := uvc main

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
endif