SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../mpi_param.mk
	include $(PARAM_FILE)
endif
include ../sample.mk

SDIR := $(CURDIR)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = sample_ipcm
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
SRCS += $(COMMON_DIR)/sample_common_peripheral.c
TARGET = sample_ipcm_asan
endif

PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common cvi_sample

MW_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) $(REQUIRES))

LIBS = $(MW_LIBS)

EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(LIBS)
EXTRA_LDFLAGS += $(MW_LIB)/libipcm.a -lpthread -lm


.PHONY : clean all
all: $(TARGET)

# $(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
# 	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
# 	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(OBJS)
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)
