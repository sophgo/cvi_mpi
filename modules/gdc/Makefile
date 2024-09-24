SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../mpi_param.mk
     include $(PARAM_FILE)
endif
#
SDIR := $(CURDIR)/src
SRCS_C := $(SDIR)/cvi_gdc.c
SRCS_C += $(SDIR)/gdc_mesh.c
SRCS_C += $(SDIR)/grid_info.c
#
ODIR    := $(CURDIR)/obj
OBJS    := $(SRCS_C:$(SDIR)/%.c=$(ODIR)/%.$(TARGET_MACHINE).$(CHIP_ARCH_L).o)
DEPS	:= $(SRCS_C:.c=.d)
#
INCS = -I$(MW_INC) -I$(MW_INTERNAL_INC) -I$(ISP_INC) -I$(SYS_INC) -I$(MSG_INC) -I$(BIN_INC) -I./include
#
TARGET_A = $(MW_LIB)/libgdc.a
TARGET_SO = $(MW_LIB)/libgdc.so

EXTRA_CFLAGS = $(INCS)

ifeq ($(MULTI_PROCESS_SUPPORT), 1)
EXTRA_CFLAGS += -DRPC_MULTI_PROCESS
endif

.PHONY : clean all
all : $(TARGET_A) $(TARGET_SO)
#
$(ODIR)/gdc_mesh.$(TARGET_MACHINE).$(CHIP_ARCH_L).o:
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -O3 -c $(SDIR)/gdc_mesh.c -o $@
	@echo [$(notdir $(CC))] $(notdir $@)

$(ODIR)/%.$(TARGET_MACHINE).$(CHIP_ARCH_L).o : $(SDIR)/%.c
	$(Q)mkdir -p $(dir $@)
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -c $< -o $@
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS)
	@$(AR) $(ARFLAGS) $(TARGET_A) $(OBJS)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $(TARGET_A))

$(TARGET_SO): $(OBJS)
	@$(LD) $(LDFLAGS) -o $(TARGET_SO) --start-group $(OBJS) --end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $(TARGET_SO))

clean:
	@rm -f $(TARGET_A) $(TARGET_SO)

clean_all: clean
	@if [ -d "$(SDIR)" ]; then \
		echo "rm -rf $(ODIR)"; \
		rm -rf $(ODIR); \
	fi

-include $(DEPS)
