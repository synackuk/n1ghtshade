CC = gcc
RM = rm
CFLAGS =  -arch x86_64 -arch arm64 -I/usr/local/include -I/opt/homebrew/include  -I. -I../../belladonna -O3 -mmacosx-version-min=10.8
LDFLAGS = -arch x86_64 -arch arm64 -L/usr/lib -L../../belladonna -O3 -mmacosx-version-min=10.8
LIBRARIES = -llzma -lbz2 -lbelladonna -lz -lcurl
FRAMEWORKS = -framework Cocoa -framework IOKit
OBJECTS = callback.o common.o dfu_enter_view.o main_view.o main.o other_options_view.o tasks_view.o 


STATIC_LIBS = libssl.a libcrypto.a libirecovery-1.0.a libimobiledevice-1.0.a libusbmuxd-2.0.a libimobiledevice-glue-1.0.a libzstd.a libzip.a libplist-2.0.a  libfragmentzip.a libgeneral.a

STATIC_DIR = ../../static/lib/

LIBRARIES += $(patsubst %,$(STATIC_DIR)/%,$(STATIC_LIBS))



all: n1ghtshade.app/Contents/MacOS/n1ghtshade

n1ghtshade.app/Contents/MacOS/n1ghtshade: $(OBJECTS)
	@echo Building $(@)
	@$(CC) $(LDFLAGS) $(LIBRARIES) $(FRAMEWORKS) -o n1ghtshade.app/Contents/MacOS/n1ghtshade $(OBJECTS)

%.o: %.m
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf n1ghtshade.app/Contents/MacOS/n1ghtshade