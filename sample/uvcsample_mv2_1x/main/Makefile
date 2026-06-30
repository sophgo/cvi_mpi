
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../../mpi_param.mk
	include $(PARAM_FILE)
endif
include ../../sample.mk

TARGET_DIR	:= $(CURDIR)
TARGET		:= $(TARGET_DIR)/sample_uvc

SEXT		:= c
SDIR		:= $(CURDIR)

INCS		+= -I$(CURDIR)/include
CFLAGS		+= -Wno-unused-parameter

UVC_PATH	=	$(CURDIR)/../uvc
UVC_INC		=	$(UVC_PATH)/include/
UVC_LIB		=	$(UVC_PATH)/lib/
SAMPLE_COMMON := ../../../sample/common

INCS		+= -I$(MW_INC) -I$(COMM_INC) -I$(ISP_INC) -I$(UVC_INC) -I$(KERNEL_INC) -I$(SAMPLE_COMMON)
LIBS 		+= -L$(MW_LIB) -Wl,-Bstatic -Wl,--start-group -lcvi_bin -lcvi_bin_isp -lcvilink -lmsg -lvenc -latomic -lvpss -lsys -lvi $(ISP_LIB) -lvo -lgdc
LIBS 		+= $(SAMPLE_COMMON)/sample_common_sys.o $(SAMPLE_COMMON)/sample_common_venc.o $(SAMPLE_COMMON)/sample_common_isp.o $(SAMPLE_COMMON)/sample_common_peripheral.o $(SAMPLE_COMMON)/sample_common_vo.o $(SAMPLE_COMMON)/sample_common_bin.o $(SAMPLE_COMMON)/sample_common_vpss.o $(SAMPLE_COMMON)/sample_common_platform.o $(SAMPLE_COMMON)/sample_common_vi.o
LIBS 		+= -L$(UVC_LIB) -lcvi_uvc
LIBS 		+= -L$(MW_LIB)/3rd/ -lini -Wl,--end-group -Wl,-Bdynamic
LIBS 		+= -lpthread -lm


# assumptions
# - SEXT is defined (c or cpp)
# - SDIR is defined
# - SRCS is optional
# - if INCS is defined, will be add to CFLAGS or CXXFLAGS
# - if LIBS is defined, will be add to LDFLAGS
SRCS ?= $(shell find $(SDIR) -type f -name "*.$(SEXT)")
SDIR := $(CURDIR)

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

CFLAGS		+= $(INCS) -fPIC -lstdc++  -D_GNU_SOURCE -g
CXXFLAGS	+= $(INCS) -fPIC -lstdc++  -D_GNU_SOURCE -g

.PHONY : all clean install
all : $(TARGET)

$(SDIR)/%.o : $(SDIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(SDIR)/%.o : $(SDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TARGET) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -Wl,-Bstatic $(LIBS) -Wl,-Bdynamic $(DYN_LIBS)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

$(INSTALL_PATH)/bin :
	mkdir -p $@

install: $(TARGET) | $(INSTALL_PATH)/bin
	cp $^ $(INSTALL_PATH)/bin

-include $(DEPS)
