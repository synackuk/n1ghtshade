CC = gcc
RM = rm
CFLAGS = -I../ 
LDFLAGS = -L/usr/lib -L/usr/local/lib -L../ -L../xpwn/lib -L/usr/local/opt/openssl/lib
LIBRARIES = -lsolanine -lcommon -ldmg -lhfs -lminizip -lxpwn -lcrypto -lz -lplist -lcurl -lpng -lfragmentzip
OBJECTS = solanine.o

all: solanine

solanine: $(OBJECTS)
	@echo Building $(@)
	@$(CC) $(LDFLAGS) $(LIBRARIES) -o solanine $(OBJECTS)

%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf solanine