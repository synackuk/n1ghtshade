TARGET = iphone:7.1:6.0
include $(THEOS)/makefiles/common.mk

ARCHS = armv7
TOOL_NAME = lockdownd

lockdownd_FILES = main.m
lockdownd_CFLAGS = -fobjc-arc

include $(THEOS_MAKE_PATH)/tool.mk

copy:: stage
	@cp $(THEOS_STAGING_DIR)/usr/bin/lockdownd .

clean::
	@$(RM) -rf lockdownd