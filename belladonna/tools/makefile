CC = gcc
RM = rm
CFLAGS = -I../
LDFLAGS = -L/usr/lib -L/usr/local/lib -L../ -L../libloader
LIBRARIES = -lusb-1.0 -lfragmentzip -lbelladonna -lloader
OBJECTS = belladonna.o

all: belladonna

belladonna: $(OBJECTS)
	@$(CC) $(LDFLAGS) $(LIBRARIES) -o belladonna $(OBJECTS)

%.o: %.c
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf belladonna