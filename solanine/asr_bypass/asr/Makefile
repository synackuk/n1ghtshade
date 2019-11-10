TARGET = iphone:6.0
include $(THEOS)/makefiles/common.mk
TOOL_NAME = asr

asr_FILES = main.c
asr_CFLAGS = -fobjc-arc

include $(THEOS_MAKE_PATH)/tool.mk

copy:: stage
	@cp -R $(THEOS_STAGING_DIR)/usr/bin/asr ../ramdisk_tar/usr/sbin/asr