CC = gcc
AR = ar
RM = rm
CFLAGS = -I/usr/local/include -I/opt/local/include -I. -I./ipsw -I./libloader
OBJECTS = libbelladonna.o exploits/checkm8/checkm8.o libloader/libloader.a libidevicerestore/libidevicerestore.a

all: libbelladonna
	@make -C tools

libbelladonna: $(OBJECTS)
	@$(AR) rs libbelladonna.a $(OBJECTS)

libloader/libloader.a: libloader
	@make -C libloader

libidevicerestore/libidevicerestore.a: libidevicerestore
	@make -C libidevicerestore

exploits/%: exploits
	@make -C exploits

%.o: %.c
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@make clean -C exploits
	@make clean -C libloader
	@make clean -C tools
	@make clean -C libidevicerestore
	@$(RM) -rf *.o
	@$(RM) -rf *.a
	@$(RM) -rf payloads