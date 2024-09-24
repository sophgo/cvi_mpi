SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../mpi_param.mk
     include $(PARAM_FILE)
endif

#
SDIR := $(CURDIR)/src
SRCS_C := $(SDIR)/cvi_msg_client.c
#
ODIR    := $(CURDIR)/obj
OBJS    := $(SRCS_C:$(SDIR)/%.c=$(ODIR)/%.$(TARGET_MACHINE).$(CHIP_ARCH_L).o)
DEPS	:= $(SRCS_C:.c=.d)
#
TARGET_A = $(MW_LIB)/libmsg.a
TARGET_SO = $(MW_LIB)/libmsg.so

INCS = -I$(MW_INC) -I$(ISP_INC) -I$(CURDIR)/include -I$(KERNEL_INC)

EXTRA_CFLAGS = $(INCS) -fpack-struct=8
EXTRA_CFLAGS += -Wno-sign-compare
EXTRA_CFLAGS += -DMMF_VERSION=\"$(shell git describe --always)\"
ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
EXTRA_CFLAGS += -D_FILE_OFFSET_BITS=64
endif

LDFLAGS  = -shared -fPIC -Wl,--gc-sections -export-dynamic -L$(MW_LIB) -L$(MW_3RD_LIB)


.PHONY : clean all
all : $(TARGET_A) $(TARGET_SO)

$(ODIR)/%.$(TARGET_MACHINE).$(CHIP_ARCH_L).o : $(SDIR)/%.c
	$(Q)mkdir -p $(dir $@)
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)   -c $<  -o $@
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS)
	@$(AR) $(ARFLAGS) $(TARGET_A) $(OBJS)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $(TARGET_A))
	@echo "$$AR_MRI" | $(AR) -M

$(TARGET_SO): $(OBJS)
	@$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $(TARGET_SO) -Wl,--start-group $(OBJS) $(LIBS) -Wl,--end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $(TARGET_SO))

clean:
	@rm -f $(TARGET_A) $(TARGET_SO)

clean_all: clean
	@if [ -d "$(SDIR)" ]; then \
		echo "rm -rf $(ODIR)"; \
		rm -rf $(ODIR); \
	fi

-include $(DEPS)
