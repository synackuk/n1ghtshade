TARGET = iphone:7.1:6.0
include $(THEOS)/makefiles/common.mk

ARCHS = armv7


TOOL_NAME = non_default_apps
non_default_apps_FILES = main.mm

include $(THEOS_MAKE_PATH)/tool.mk

copy:: stage
	@cp $(THEOS_STAGING_DIR)/usr/bin/non_default_apps .

clean::
	@$(RM) -rf non_default_apps