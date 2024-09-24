USE_SONIC_LIB = no
USE_ION_MEM = no
USE_ALSA = no
USE_TINYALSA = yes
USE_PTHREAD = yes
DBG_PRINT = yes
USE_NE10 = yes
USE_NEON_SIMD = no
USE_SYSTRACE = no
USE_SYS_GLOBAL_LOG = no
USE_ALGO = no
USE_CVI_AEC = yes
USE_QUICK_AACDEC_BUILD = yes
USE_CYCLEBUFFER = yes
MULTI_PROCESS_SUPPORT_AUDIO = no
USE_SHARE_MEM_IN_AUDIO_RPC = no
USE_AiMultiMic = no
USE_HEAP_REPLACE_STACK = yes
USE_CVITEK_SOFTWARE_AEC = yes
USE_NEXT_SSP_DUAL = yes
USE_SPEED = no
USE_RESAEC = no
ifneq ($(CONFIG_ENABLE_SDK_ASAN), y)
ifeq ($(CHIP_ARCH), $(filter $(CHIP_ARCH), CV181X CV180X))
USE_CVIAUDIO_STATIC = yes
else
USE_CVIAUDIO_STATIC = no
endif
endif