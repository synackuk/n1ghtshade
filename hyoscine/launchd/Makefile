include $(THEOS)/makefiles/common.mk

ARCHS = armv7
TOOL_NAME = launchd
launchd_FILES = start.S main.c common.c syscalls.S
launchd_CFLAGS = -I. -fno-stack-protector -nostdlib
launchd_LDFLAGS = -fno-stack-protector -nostdlib

launchd_CODESIGN_FLAGS=-Sent.xml
include $(THEOS_MAKE_PATH)/tool.mk


copy:: stage
	@cp -r $(THEOS_STAGING_DIR)/usr/bin/launchd .

clean::
	@$(RM) -rf launchd