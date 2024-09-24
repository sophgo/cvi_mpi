SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../mpi_param.mk
	include $(PARAM_FILE)
endif
include ../sample.mk

SDIR := $(CURDIR)
SRCS = $(wildcard $(SDIR)/*.c)
SRCS +=  $(COMMON_DIR)/sample_common_vo.c $(COMMON_DIR)/sample_common_sys.c $(COMMON_DIR)/sample_common_platform.c $(COMMON_DIR)/sample_common_vpss.c $(COMMON_DIR)/sample_common_venc.c
INCS = -I$(MW_INC) -I$(ISP_INC) -I../common/ -I$(KERNEL_INC) -I$(PANEL_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = sample_vdecvo
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET = sample_vdecvo_asan
endif

PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common cvi_sample cvi_vdec
MW_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) $(REQUIRES))

LIBS = $(MW_LIBS)
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
LIBS += -lnanomsg
endif

EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(LIBS) -lini -lm
EXTRA_LDFLAGS += $(MW_LIB)/libmsg.a $(MW_LIB)/libcvilink.a -lpthread

.PHONY : clean all
all: $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@) $(EXTRA_CFLAGS)

$(TARGET): $(OBJS) $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) -lsys $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)