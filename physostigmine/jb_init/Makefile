include $(THEOS)/makefiles/common.mk

TOOL_NAME = jb_init

jb_init_FILES = main.m
jb_init_CFLAGS = -fobjc-arc

include $(THEOS_MAKE_PATH)/tool.mk

copy:: stage
	@cp $(THEOS_STAGING_DIR)/usr/bin/jb_init ../../hyoscine/modules
