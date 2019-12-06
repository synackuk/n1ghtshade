CC = gcc
RM = rm
CFLAGS = -I../../belladonna
LDFLAGS = -L/usr/lib -L/usr/local/lib -L../../belladonna/ -L../../belladonna/libloader -L../../belladonna/libidevicerestore -L/usr/local/opt/openssl/lib
LIBRARIES = -lcrypto -lz -lzip -lplist -lcurl -lfragmentzip -lusb-1.0 -lbelladonna -lloader -lirecovery -limobiledevice -lidevicerestore
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