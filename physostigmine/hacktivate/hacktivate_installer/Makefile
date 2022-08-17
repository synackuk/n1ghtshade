TARGET = iphone:7.1:6.0
include $(THEOS)/makefiles/common.mk

ARCHS = armv7
TOOL_NAME = hacktivate_installer

hacktivate_installer_FILES = main.m
hacktivate_installer_CFLAGS = -fobjc-arc
hacktivate_installer_FRAMEWORKS = UIKit

include $(THEOS_MAKE_PATH)/tool.mk

copy:: stage
	@cp $(THEOS_STAGING_DIR)/usr/bin/hacktivate_installer .

clean::
	@$(RM) -rf hacktivate_installer