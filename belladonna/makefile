CC = gcc
AR = ar
RM = rm
CFLAGS = -I/usr/local/include -I/opt/local/include -I. -I./ipsw -I./libloader -mmacosx-version-min=10.12
OBJECTS = libbelladonna.o exploits/checkm8/checkm8.o libloader/libloader.a libidevicerestore/libidevicerestore.a
PAYLOADS = payloads/atropine.h payloads/hyoscine.h

all: libbelladonna.a
	@make -C tools

libbelladonna.a: $(PAYLOADS) $(OBJECTS)
	@echo Building $(@)
	@echo AR rs $(@) $(OBJECTS)
	@$(AR) rs $(@) $(OBJECTS)

libloader/libloader.a: libloader
	@echo Building $(@)
	@make -C libloader

libidevicerestore/libidevicerestore.a: libidevicerestore
	@echo Building $(@)
	@make -C $(<)

exploits/%: exploits
	@echo Building $(@)
	@make -C $(<)

%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

payloads/%.h: $(patsubst %,../%,%)
	@echo Building $(<)
	@make -C $(<)

clean:
	@make clean -C exploits
	@make clean -C libloader
	@make clean -C tools
	@make clean -C libidevicerestore
	@$(RM) -rf *.o
	@$(RM) -rf *.a
	@$(RM) -rf payloads