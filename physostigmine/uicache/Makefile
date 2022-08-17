TARGET = iphone:7.1:6.0
include $(THEOS)/makefiles/common.mk

ARCHS = armv7


TOOL_NAME = uicache
uicache_FILES = main.mm

include $(THEOS_MAKE_PATH)/tool.mk

copy:: stage
	@cp $(THEOS_STAGING_DIR)/usr/bin/uicache .

clean::
	@$(RM) -rf uicache