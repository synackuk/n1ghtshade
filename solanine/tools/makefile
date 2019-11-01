CC = gcc
RM = rm
CFLAGS = -I../ 
LDFLAGS = -L/usr/lib -L/usr/local/lib -L../ -L../xpwn/lib -L/usr/local/opt/openssl/lib
LIBRARIES = -lsolanine -lcommon -ldmg -lhfs -lminizip -lxpwn -lcrypto -lz -lplist -lcurl -lpng -lfragmentzip
OBJECTS = solanine.o

all: solanine

solanine: $(OBJECTS)
	@$(CC) $(LDFLAGS) $(LIBRARIES) -o solanine $(OBJECTS)

%.o: %.c
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf solanine