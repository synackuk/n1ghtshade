CC = gcc
RM = rm
CFLAGS =  -arch x86_64 -arch arm64 -I/usr/local/include -I/opt/homebrew/include  -I../ -I../../static/include -O3
LDFLAGS = -arch x86_64 -arch arm64 -L/usr/lib -L../ -O3
FRAMEWORKS = -framework IOKit -framework Cocoa
LIBRARIES = -llzma -lbz2 -lbelladonna -lz -lcurl

OBJECTS = belladonna.o

STATIC_LIBS = libssl.a libcrypto.a libirecovery-1.0.a libimobiledevice-1.0.a libusbmuxd-2.0.a libimobiledevice-glue-1.0.a libzstd.a libzip.a libplist-2.0.a  libfragmentzip.a libgeneral.a

STATIC_DIR = ../../static/lib/

LIBRARIES += $(patsubst %,$(STATIC_DIR)/%,$(STATIC_LIBS))

all: belladonna

belladonna: $(OBJECTS)
	@echo Building $(@)
	@$(CC) $(LDFLAGS) $(FRAMEWORKS) $(LIBRARIES) -o belladonna $(OBJECTS)

%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf belladonna