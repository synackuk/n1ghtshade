CC = gcc
RM = rm
CFLAGS = -I. -I../../solanine -I../../belladonna
LDFLAGS = -L/usr/local/lib -L/usr/lib -L../../solanine/xpwn/lib -L../../solanine/ -L../../belladonna/ -L../../belladonna/libloader -L../../belladonna/libidevicerestore -L/usr/local/opt/openssl/lib
LIBRARIES = -lsolanine -lcommon -ldmg -lhfs -lminizip -lxpwn -lcrypto -lz -lzip -lplist -lcurl -lpng -lfragmentzip -lusb-1.0 -lbelladonna -lloader -lirecovery -limobiledevice -lidevicerestore
FRAMEWORKS = -framework Cocoa
OBJECTS = main.o tethered_boot_view.o main_view.o common.o

.PHONY: n1ghtshade.app

all: n1ghtshade.app

n1ghtshade.app: $(OBJECTS)
	@echo Building $(@)
	@$(CC) $(LDFLAGS) $(LIBRARIES) $(FRAMEWORKS) -o n1ghtshade.app/Contents/MacOS/n1ghtshade $(OBJECTS)

%.o: %.m
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf n1ghtshade.app/Contents/MacOS/n1ghtshade