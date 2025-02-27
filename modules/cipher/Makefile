SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../../mpi_param.mk
     include $(PARAM_FILE)
endif

OPENSSL_PATH=../../3rdparty/openssl
#
#SDIR := $(CURDIR)/src
SDIR := $(CURDIR)
SRCS_C := $(SDIR)/cvi_unf_cipher.c

INCS = -I$(MW_INC) -I$(KERNEL_INC) -I$(SYS_INC) -I$(OPENSSL_PATH)/include -I.
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
ifeq ($(TARGET_MACHINE), arm-linux-gnueabihf)
LIBS = $(OPENSSL_PATH)/libcrypto-arm.a
LOCAL_OPT_LEVEL = -O3
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-gnu)
LIBS = $(OPENSSL_PATH)/libcrypto-riscv64.a
LOCAL_OPT_LEVEL = -O3
else ifeq ($(TARGET_MACHINE), riscv64-unknown-linux-musl)
LIBS = $(OPENSSL_PATH)/libcrypto-musl_riscv64.a
LOCAL_OPT_LEVEL = -O3
else
LIBS = $(OPENSSL_PATH)/libcrypto.a
LOCAL_OPT_LEVEL = -Os
endif
TARGET_A = $(abspath  $(MW_LIB)/libcipher.a)
TARGET_SO = $(abspath  $(MW_LIB)/libcipher.so)
TMP_A = cipher_tmp.a

EXTRA_CFLAGS = $(INCS)

.PHONY : clean all
all : $(TARGET_A) $(TARGET_SO)

CFLAGS += -ggdb $(LOCAL_OPT_LEVEL) -fno-omit-frame-pointer
CIPHER_LDFLAGS := $(filter-out --gc-sections,${LDFLAGS})
CIPHER_LDFLAGS +=  -ldl

$(ODIR)/%.$(TARGET_MACHINE).o : $(SDIR)/%.c
	$(Q)mkdir -p $(dir $@)
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS)   -c $<  -o $@
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS)
	@$(LD) -relocatable $(EXTRA_LDFLAGS) $(OBJS) $(LIBS) -o cipher.o
	@SYMS=$$(${NM} cipher.o | awk '$$3 ~ /./ { print $$3 }' | sort -u | grep -v '^CVI_' | sed 's/^/-L /'); \
		$(OBJCOPY) $$SYMS cipher.o cipher.o
	@$(AR) $(ARFLAGS) $(TARGET_A) cipher.o
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $(TARGET_A))

$(TARGET_SO): $(OBJS)
	@$(CC) -Wl,--version-script=libcipher.version $(CIPHER_LDFLAGS) $(EXTRA_LDFLAGS) -o $(TARGET_SO) -Wl,--start-group $(OBJS) $(LIBS) -Wl,--end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $(TARGET_SO))

clean:
	@rm -rf $(ODIR) $(TARGET_A) $(TARGET_SO)

-include $(DEPS)
