TARGET = iphone:6.0

include $(THEOS)/makefiles/common.mk
LIBRARY_NAME = asr_bypass

asr_bypass_FILES = impHook.c asr_bypass.c
asr_bypass_CFLAGS = -fobjc-arc
asr_bypass_LDFLAGS = -Xlinker -no_warn_inits

include $(THEOS_MAKE_PATH)/library.mk

copy:: stage
	@cp -R $(THEOS_STAGING_DIR)/usr/lib/asr_bypass.dylib ../ramdisk_tar/usr/lib/asr_bypass.dylib