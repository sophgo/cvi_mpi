SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include $(BUILD_PATH)/.config

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c) $(wildcard $(SDIR)/src/*.c)
INCS = -I$(MW_INC) -I$(ISP_INC) -I$(VI_INC) -I$(KERNEL_INC) -I./inc $(COMM_INC)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

INCS = -I$(MW_INC) -I$(ISP_INC) -I$(MW_INC)/sensor -I./inc

TARGET = sample_sensor

ifeq ($(OS_TYPE), DUAL_OS)
LIBS += -lvi -lvpss -lsys -lini -lsensor -lisp -lawb -lae -laf
else
LIBS = -lvi -lini -lsns_full -lsensor -lgdc -lsys -lisp -lawb -lae -laf -lisp_algo -lcvi_bin -lvo -lvpss
endif

EXTRA_CFLAGS += $(INCS)
EXTRA_CFLAGS += -ffunction-sections -fdata-sections

EXTRA_LDFLAGS += $(LIBS) -lpthread -lm -latomic

ifneq ($(CONFIG_ENABLE_SDK_ASAN), y)
ELFFLAGS += -static
endif

.PHONY : clean all

all : $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS) $(MW_LIB)/libsys.a $(MW_LIB)/libmipi.a $(MW_LIB)/libsensor.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(MW_LIB)/libsys.a \
	$(MW_LIB)/libmipi.a $(MW_LIB)/libsensor.a -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(TARGET)

