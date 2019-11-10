TARGET = iphone:6.0
include $(THEOS)/makefiles/common.mk
ARCHS = armv7
TOOL_NAME = kill_software_update
kill_software_update_FILES = main.mm

include $(THEOS_MAKE_PATH)/tool.mk

copy:: stage
	@cp $(THEOS_STAGING_DIR)/usr/bin/kill_software_update ../../hyoscine/modules