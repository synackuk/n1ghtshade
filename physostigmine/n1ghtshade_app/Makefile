TARGET = iphone:6.0
include $(THEOS)/makefiles/common.mk

APPLICATION_NAME = n1ghtshade
TOOL_NAME = Bootstrap
APPLICATION_NAME = n1ghtshade
n1ghtshade_FILES = main.m N1AppDelegate.m N1RootViewController.m
n1ghtshade_FRAMEWORKS = UIKit CoreGraphics
n1ghtshade_CFLAGS = -I.
Bootstrap_FILES = Bootstrap.m
Bootstrap_INSTALL_PATH = /Applications/n1ghtshade.app/

include $(THEOS_MAKE_PATH)/application.mk
include $(FW_MAKEDIR)/tool.mk

after-n1ghtshade-stage::
	@$(FAKEROOT) chmod 6755 $(THEOS_SHARED_BUNDLE_RESOURCE_PATH)n1ghtshade

copy:: stage
	@cp -r $(THEOS_STAGING_DIR)/Applications/n1ghtshade.app ../../hyoscine/modules