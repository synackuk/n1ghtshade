TARGET = iphone:7.1:6.0
SYSROOT= $(THEOS)/sdks/iPhoneOS7.1.sdk

include $(THEOS)/makefiles/common.mk

ARCHS = armv7

TOOL_NAME = jailbreakd

jailbreakd_FILES = main.c server.c
jailbreakd_FILES += commands/commands.c commands/menu_commands.m
jailbreakd_FILES += drivers/fb/fb.m drivers/fb/display.c drivers/fb/image.c drivers/fb/shape.c drivers/fb/text.c
jailbreakd_FILES += drivers/usb/usb.c
jailbreakd_FILES += drivers/tcp/tcp.c
jailbreakd_FILES += drivers/watchdog/watchdog.c

jailbreakd_CFLAGS = -Iapple_headers -I.
jailbreakd_CODESIGN_FLAGS = -Sent.xml
jailbreakd_INSTALL_PATH = /usr/local/bin
jailbreakd_FRAMEWORKS = Foundation IOKit
jailbreakd_PRIVATE_FRAMEWORKS = IOMobileFramebuffer IOSurface


include $(THEOS_MAKE_PATH)/tool.mk

copy:: stage
	@cp -r $(THEOS_STAGING_DIR)/usr/local/bin/jailbreakd .

clean::
	@$(RM) -rf jailbreakd