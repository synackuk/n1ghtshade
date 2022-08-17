TARGET = iphone:7.1:6.0
include $(THEOS)/makefiles/common.mk

ARCHS = armv7
TOOL_NAME = jb_init

jb_init_FILES = main.m remount.m kernel.m launch_daemons.c
jb_init_CFLAGS = -fobjc-arc -Wno-for-loop-analysis -I./ -Wno-visibility
jb_init_FRAMEWORKS = CoreFoundation Foundation UIKit
jb_init_CODESIGN_FLAGS=-Sent.xml
include $(THEOS_MAKE_PATH)/tool.mk

copy:: stage
	@cp $(THEOS_STAGING_DIR)/usr/bin/jb_init .

clean::
	@$(RM) -rf jb_init