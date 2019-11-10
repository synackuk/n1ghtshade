TARGET = iphone:6.0

include $(THEOS)/makefiles/common.mk
LIBRARY_NAME = hacktivate

hacktivate_FILES = hacktivate.c
hacktivate_CFLAGS = -fobjc-arc
hacktivate_LDFLAGS = -Xlinker -no_warn_inits
hacktivate_INSTALL_PATH = /

include $(THEOS_MAKE_PATH)/library.mk

copy:: stage
	@cp $(THEOS_STAGING_DIR)/hacktivate.dylib ../../../hyoscine/modules