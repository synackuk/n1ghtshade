CC = gcc
RM = rm
CFLAGS = -I../../solanine -I../../belladonna
LDFLAGS = -L/usr/lib -L/usr/local/lib -L../../solanine/xpwn/lib -L../../solanine/ -L../../belladonna/ -L../../belladonna/libloader -L../../belladonna/libidevicerestore -L/usr/local/opt/openssl/lib
LIBRARIES = -lsolanine -lcommon -ldmg -lhfs -lminizip -lxpwn -lcrypto -lz -lzip -lplist -lcurl -lpng -lfragmentzip -lusb-1.0 -lbelladonna -lloader -lirecovery -limobiledevice -lidevicerestore
OBJECTS = n1ghtshade.o

all: n1ghtshade

n1ghtshade: $(OBJECTS)
	@echo Building $(@)
	@$(CC) $(LDFLAGS) $(LIBRARIES) -o n1ghtshade $(OBJECTS)

%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf n1ghtshade