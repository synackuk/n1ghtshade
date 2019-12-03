CC = gcc
AR = ar
RM = rm
CFLAGS = -I/usr/local/include -I/opt/local/include -I. -I./xpwn/includes -I/usr/local/opt/openssl/include 
OBJECTS = libsolanine.o

all: asr_bypass.h libsolanine.a
	@make -C xpwn
	@make -C tools

asr_bypass.h:
	@echo Building $(@)
	@make -C asr_bypass

libsolanine.a: $(OBJECTS)
	@echo Building $(@)
	@$(AR) rs $(@) $(OBJECTS)

%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@make clean -C tools
	@make clean -C asr_bypass
	@make clean -C xpwn
	@$(RM) -rf *.o
	@$(RM) -rf *.a