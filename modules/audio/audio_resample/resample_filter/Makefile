.PHONY: CREATE_DIR
ifeq ($(PARAM_FILE), )
PARAM_FILE:=../../../../../mpi_param.mk
include $(PARAM_FILE)
endif

SRCS = $(wildcard $(CURDIR)/*.c)
EXTRA_SRC =
SRCS += $(EXTRA_SRC)
INCS = -I$(MW_INC) -I$(KERNEL_INC)
INCS += -I ../../../include  -I .
OBJS = $(SRCS:.c=.o)

TARGET = $(MW_LIB)/libcvi_RES1.a
TARGET_S = $(MW_LIB)/libcvi_RES1.so

ARFLAGS = rcs
ARFLAGS_S = -shared -fPIC -o
CFLAGS =  -O3 -fPIC
CFLAGS += -DOUTSIDE_SPEEX
CFLAGS += -D__FILENAME__=\"$(notdir $<)\"

#for execution bin --start
EXENAME :=  audio_resample_ver3
LIB_DIR = -L$(CURDIR)/
LIBS +=  -lm -lpthread
OBJS_EXE := $(SRCS:%.c=%.o)
#for execution bin --end

OBJECTPATHS_SRC=$(addprefix $(CURDIR)/,$(notdir $(OBJS)))

BUILD_LIST =  $(TARGET)
#BUILD_LIST += $(EXENAME)
all : $(BUILD_LIST)


$(OBJS):
	$(CC) $(SRCS) $(CFLAGS) -c  $(INCS)

$(TARGET) : $(OBJS)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJECTPATHS_SRC)
	$(CC) $(ARFLAGS_S) $(TARGET_S) $(OBJECTPATHS_SRC)

$(EXENAME) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^  $(LIB_DIR) $(LIBS)  $(INCS)


.PHONY : clean
clean:
	rm -f $(OBJS) $(TARGET) $(TARGET_S) $(OBJECTPATHS_SRC)



