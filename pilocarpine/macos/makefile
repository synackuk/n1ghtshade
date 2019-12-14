CC = gcc
STRIP = strip
RM = rm
INSTALL_NAME_TOOL = install_name_tool
CFLAGS = -I. -I../../belladonna -mmacosx-version-min=10.12
LDFLAGS = -L/usr/local/lib -L/usr/lib -L../../belladonna/ -L../../belladonna/libloader -L../../belladonna/libidevicerestore -L/usr/local/opt/openssl/lib -mmacosx-version-min=10.12
LIBRARIES = -lcrypto -lz -lzip -lplist -lcurl -lfragmentzip -lusb-1.0 -lbelladonna -lloader -lirecovery -limobiledevice -lidevicerestore
FRAMEWORKS = -framework Cocoa
OBJECTS = main.o restore_view.o jailbreak_view.o tethered_boot_view.o main_view.o common.o

.PHONY: n1ghtshade.app

all: n1ghtshade.app

n1ghtshade.app: $(OBJECTS)
	@echo Building $(@)
	@$(CC) $(LDFLAGS) $(LIBRARIES) $(FRAMEWORKS) -o n1ghtshade.app/Contents/MacOS/n1ghtshade $(OBJECTS)
	@$(INSTALL_NAME_TOOL) -change /usr/local/opt/openssl/lib/libcrypto.1.0.0.dylib @loader_path/../Frameworks/libcrypto.1.0.0.dylib n1ghtshade.app/Contents/MacOS/n1ghtshade
	@$(INSTALL_NAME_TOOL) -change /usr/local/opt/libzip/lib/libzip.5.dylib @loader_path/../Frameworks/libzip.5.0.dylib n1ghtshade.app/Contents/MacOS/n1ghtshade
	@$(INSTALL_NAME_TOOL) -change /usr/local/lib/libplist.3.dylib @loader_path/../Frameworks/libplist.3.dylib n1ghtshade.app/Contents/MacOS/n1ghtshade
	@$(INSTALL_NAME_TOOL) -change /usr/local/opt/libfragmentzip/lib/libfragmentzip.0.dylib @loader_path/../Frameworks/libfragmentzip.0.dylib n1ghtshade.app/Contents/MacOS/n1ghtshade
	@$(INSTALL_NAME_TOOL) -change @@HOMEBREW_PREFIX@@/opt/libusb/lib/libusb-1.0.0.dylib @loader_path/../Frameworks/libusb-1.0.0.dylib n1ghtshade.app/Contents/MacOS/n1ghtshade
	@$(INSTALL_NAME_TOOL) -change /usr/local/lib/libirecovery.2.dylib @loader_path/../Frameworks/libirecovery.2.dylib n1ghtshade.app/Contents/MacOS/n1ghtshade
	@$(INSTALL_NAME_TOOL) -change /usr/local/lib/libimobiledevice.6.dylib @loader_path/../Frameworks/libimobiledevice.6.dylib n1ghtshade.app/Contents/MacOS/n1ghtshade
	@echo STRIP n1ghtshade.app/Contents/MacOS/n1ghtshade
	@$(STRIP) n1ghtshade.app/Contents/MacOS/n1ghtshade
%.o: %.m
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf n1ghtshade.app/Contents/MacOS/n1ghtshade