SHELL = /bin/bash

STATIC ?= 0

ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../mpi_param.mk
	include $(PARAM_FILE)
endif
include ../sample.mk

SDIR := $(CURDIR)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(COMMON_DIR)/ -I. -I$(KERNEL_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = sample_cipher
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET = sample_cipher_asan
endif

EXTRA_CFLAGS = $(INCS)

ldflags-y += -lmisc -lsys -lmsg -lcvilink
ldflags-y += -lcvi_bin -lcvi_bin_isp $(ISP_LIB)
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
ldflags-y += -lnanomsg -lvenc -lvdec
endif
ldflags-y += -lvi -lvpss -lvo -lrgn -lgdc -lpthread -latomic -ldl

ifeq ($(SAMPLE_STATIC),1)
LIB_DEP := $(MW_LIB)/libsys.a $(MW_LIB)/libmsg.a $(MW_LIB)/libcvilink.a
else
LIB_DEP := $(MW_LIB)/libsys.so $(MW_LIB)/libmsg.so $(MW_LIB)/libcvilink.so
endif

.PHONY : clean all
all : $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(OBJS) $(LIB_DEP)
	@$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(ldflags-y)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)
