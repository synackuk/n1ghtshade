CC = gcc
RM = rm
CFLAGS = -I../
LDFLAGS = -L/usr/lib -L/usr/local/lib -L../ -L../libloader -L../libidevicerestore -L/usr/local/opt/openssl/lib
LIBRARIES = -lusb-1.0 -lfragmentzip -lbelladonna -lloader -lz -lzip -lirecovery -limobiledevice -lplist -lcrypto -lcurl -lidevicerestore
OBJECTS = belladonna.o

all: belladonna

belladonna: $(OBJECTS)
	@echo Building $(@)
	@$(CC) $(LDFLAGS) $(LIBRARIES) -o belladonna $(OBJECTS)

%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf belladonna