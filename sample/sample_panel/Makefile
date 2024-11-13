SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../mpi_param.mk
	include $(PARAM_FILE)
endif
include ../sample.mk

SDIR := $(CURDIR)
SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(PANEL_INC) -I$(COMMON_DIR)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = sample_panel
ifeq ($(CONFIG_ENABLE_SDK_ASAN), y)
TARGET = sample_panel_asan
endif

LIBS = -lvi -lvo -lsys -lmipi_tx -lmisc

ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
LIBS += -lnanomsg
LIBS += -lvenc -lvdec -lmisc
endif

CFLAGS += $(KBUILD_DEFINES)
EXTRA_CFLAGS = $(INCS)
EXTRA_LDFLAGS = $(LIBS) -lmsg -lcvilink -lpthread
EXTRA_LDFLAGS += $(MW_LIB)/libvi.a $(MW_LIB)/libvo.a $(MW_LIB)/libsys.a $(MW_LIB)/libmipi_tx.a $(MW_LIB)/libmsg.a $(MW_LIB)/libcvilink.a -lpthread

.PHONY : clean all
all: $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)
