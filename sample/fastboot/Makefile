SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../mpi_param.mk
	include $(PARAM_FILE)
endif
include ../sample.mk

SDIR := $(CURDIR)
SRCS = $(wildcard $(SDIR)/*.c)

VENC_SRCS = $(SDIR)/sample_venc.c

SAMPLE_VENC_INC=$(SDIR)/include
SAMPLE_VDEC_INC=$(SDIR)/../vdec/include
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(COMM_INC) -I$(KERNEL_INC) -I$(SAMPLE_VENC_INC) -I$(SAMPLE_VDEC_INC) -I../common -I$(SYS_INC)

OBJS = $(SRCS:.c=.o)

DEPS = $(SRCS:.c=.d)

TARGET_VENC = sample_fastboot

PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common cvi_sample
MW_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) $(REQUIRES))

LIBS = $(MW_LIBS)
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
LIBS += -lnanomsg
endif

EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(LIBS)
EXTRA_LDFLAGS += $(MW_LIB)/libmsg.a $(MW_LIB)/libcvilink.a $(MW_LIB)/libvi.a $(MW_LIB)/libvpss.a \
				 $(MW_LIB)/libvo.a $(MW_LIB)/librgn.a $(MW_LIB)/libgdc.a -lpthread
EXTRA_LDFLAGS_WITH_ASAN = -Wl,-Bdynamic $(LIBS) -lpthread

ifeq ($(DEBUG), 1)
EXTRA_LDFLAGS += -g -O0
EXTRA_LDFLAGS_WITH_ASAN += -g -O0
endif

EXTRA_LDFLAGS_WITH_ASAN += -fno-omit-frame-pointer -fsanitize-recover=address -lasan

.PHONY : clean all

all : $(TARGET_VENC) $(TARGET_VENC_WITH_ASAN)


$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_VENC) : $(VENC_OBJS) $(OBJS) $(MW_LIB)/libvenc.a $(MW_LIB)/libvi.a $(MW_LIB)/libvpss.a \
				 $(MW_LIB)/libvo.a $(MW_LIB)/librgn.a $(MW_LIB)/libgdc.a $(MW_LIB)/libsys.a
	@$(CXX) -o $@ $(OBJS) $(VENC_OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

# $(TARGET_VCODEC) : $(VCODEC_OBJS) $(OBJS) $(MW_LIB)/libvenc.a $(MW_LIB)/libvdec.a $(MW_LIB)/libvi.a $(MW_LIB)/libvpss.a \
				 $(MW_LIB)/libvo.a $(MW_LIB)/librgn.a $(MW_LIB)/libgdc.a $(MW_LIB)/libsys.a
# 	@$(CXX) -o $@ $(OBJS) $(VCODEC_OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS)
# 	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

$(TARGET_VENC_WITH_ASAN) : $(VENC_OBJS) $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(VENC_OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS_WITH_ASAN)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

# $(TARGET_VCODEC_WITH_ASAN) : $(VCODEC_OBJS) $(OBJS)
# 	@$(CXX) -o $@ $(OBJS) $(VCODEC_OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS_WITH_ASAN)
# 	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(VENC_OBJS)  $(DEPS) $(VENC_DEPS)  $(TARGET_VENC) $(TARGET_VENC_WITH_ASAN)

-include $(VENC_DEPS) $(VCODEC_DEPS) $(DEPS)
