CC = gcc
AR = ar
RM = rm
CFLAGS = -I/usr/local/include -I/opt/local/include -I. -I./xpwn/includes -I/usr/local/opt/openssl/include 
OBJECTS = libsolanine.o

all: asr_bypass.h libsolanine
	@make -C xpwn
	@make -C tools

asr_bypass.h:
	@make -C asr_bypass

libsolanine: $(OBJECTS)
	@$(AR) rs libsolanine.a $(OBJECTS)

%.o: %.c
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@make clean -C tools
	@make clean -C asr_bypass
	@make clean -C xpwn
	@$(RM) -rf *.o
	@$(RM) -rf *.a