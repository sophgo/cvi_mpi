SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../mpi_param.mk
	include $(PARAM_FILE)
endif
include ../sample.mk

SDIR := $(CURDIR)
SRCS = $(wildcard $(SDIR)/*.c) $(wildcard $(SDIR)/src/*.c)
SRCS += $(COMMON_DIR)/sample_common_vi.c $(COMMON_DIR)/sample_common_isp.c $(COMMON_DIR)/sample_common_sys.c $(COMMON_DIR)/sample_common_platform.c $(COMMON_DIR)/sample_common_bin.c
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
SRCS += $(COMMON_DIR)/sample_common_vo.c $(COMMON_DIR)/sample_common_vpss.c $(COMMON_DIR)/sample_common_peripheral.c
endif
INCS = -I$(MW_INC) -I$(ISP_INC) -I../common/ -I$(KERNEL_INC) -I./inc -I./../../internal_include -I$(MW_PATH)/3rdparty/inih
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = sensor_test
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET = sensor_test_asan
endif

PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common cvi_sample

MW_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) $(REQUIRES))

LIBS = $(MW_LIBS)
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
LIBS += -lnanomsg
endif

EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(LIBS) -lm -lini
EXTRA_LDFLAGS += $(MW_LIB)/libmsg.a $(MW_LIB)/libcvilink.a $(MW_LIB)/libvi.a $(MW_LIB)/libvo.a $(MW_LIB)/libvpss.a -lpthread

# IVE_SUPPORT = 1
ifeq ($(IVE_SUPPORT), 1)
CFLAGS += -DIVE_SUPPORT

IVE_PATH = $(MW_PATH)/../install/soc_cv1835_wevb_0002a_spinand/tpu_64/cvitek_ive_sdk
EXTRA_CFLAGS += -I$(IVE_PATH)/include/ive
EXTRA_LDFLAGS += -L$(IVE_PATH)/lib -lcvi_ive_tpu-static

TPU_PATH = $(MW_PATH)/../install/soc_cv1835_wevb_0002a_spinand/tpu_64/cvitek_tpu_sdk
EXTRA_CFLAGS += -I$(TPU_PATH)/include
EXTRA_LDFLAGS += -L$(TPU_PATH)/lib -lcviruntime-static -lcvimath-static -lcvikernel-static -lcnpy -lglog -lz
endif

.PHONY : clean all
all: $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(OBJS) $(ISP_OBJ) $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) -lsys $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
