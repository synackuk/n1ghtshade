TARGET = iphone:7.1:6.0
include $(THEOS)/makefiles/common.mk

ARCHS = armv7
LIBRARY_NAME = hacktivate

hacktivate_FILES = hacktivate.c
hacktivate_CFLAGS = -fobjc-arc 
hacktivate_LDFLAGS = -dynamiclib
hacktivate_INSTALL_PATH = /

include $(THEOS_MAKE_PATH)/library.mk

copy:: stage
	@cp $(THEOS_STAGING_DIR)/hacktivate.dylib .

clean::
	@$(RM) -rf hacktivate.dylib