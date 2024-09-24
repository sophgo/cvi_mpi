SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../mpi_param.mk
	include $(PARAM_FILE)
endif
include ../sample.mk

SDIR := $(CURDIR)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(COMMON_DIR) -I$(KERNEL_INC) -I$(SYS_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

COMM_SRC := $(COMMON_DIR)/sample_common_datafifo.c
COMM_OBJ := $(COMM_SRC:%.c=%.o)

TARGET = sample_reader
LIBS = -lcvilink -lsys

EXTRA_CFLAGS += $(DEFS) $(INCS)
EXTRA_CFLAGS += -ffunction-sections -fdata-sections

EXTRA_LDFLAGS += $(LIBS) -lpthread

ifneq ($(CONFIG_ENABLE_SDK_ASAN), y)
ELFFLAGS += -static
endif

all : $(TARGET)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

	#/mnt/workspace/mars_daily_32_2/host-tools/gcc/gcc-linaro-12.0.0-2021.11-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc -fanalyzer -fdump-analyzer-callgraph $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(COMM_OBJ) $(OBJS) $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(COMM_OBJ) $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

.PHONY : clean
clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_OBJ) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
