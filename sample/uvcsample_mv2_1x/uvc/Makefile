
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../../mpi_param.mk
	include $(PARAM_FILE)
endif
include ../../sample.mk

TARGET_DIR	:= $(CURDIR)/lib
TARGET_A	:= $(TARGET_DIR)/libcvi_uvc.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_uvc.so

############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include
CFLAGS		+= -Wno-unused-parameter

INCS 		+= -I$(MW_INC) -I$(ISP_INC) -I$(COMM_INC) -I$(KERNEL_INC) -I../../../sample/common

SEXT		:= c
SDIR		:= $(CURDIR)/src

# include $(BUILD_PATH)/.config

EXTRA_CFLAGS += $(KBUILD_DEFINES)

SRCS ?= $(shell find $(SDIR) -type f -name "*.$(SEXT)")
ODIR := $(patsubst $(SRCTREE)/%,$(OBJTREE)/%,$(SDIR))
OBJS := $(addprefix $(ODIR)/,$(notdir $(SRCS:.$(SEXT)=.o)))
DEPS := $(addprefix $(ODIR)/,$(notdir $(SRCS:.$(SEXT)=.d)))

CFLAGS		+= $(INCS) -fPIC -lstdc++  -D_GNU_SOURCE -g
CXXFLAGS	+= $(INCS) -fPIC -lstdc++  -D_GNU_SOURCE -g
ARFLAGS		= rcs
LDFLAGS_SO	= -shared -fPIC

.PHONY : all clean install
all : $(TARGET_A) $(TARGET_SO)

$(ODIR) :
	mkdir -p $@

$(ODIR)/%.o : $(SDIR)/%.c | $(ODIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(ODIR)/%.o : $(SDIR)/%.cpp | $(ODIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TARGET_DIR) :
	mkdir -p $@

$(TARGET_A) : $(OBJS) | $(TARGET_DIR)
	$(AR) $(ARFLAGS) $@ $^

$(TARGET_SO) : $(OBJS) | $(TARGET_DIR)
	$(CC) $(LDFLAGS_SO) -o $@ $^

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO)

$(INSTALL_PATH)/lib :
	mkdir -p $@

install: $(TARGET_A) $(TARGET_SO) | $(INSTALL_PATH)/lib
	cp $^ $(INSTALL_PATH)/lib

-include $(DEPS)
