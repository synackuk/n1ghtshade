CC = gcc
AR = ar
RM = rm
CP = cp
CFLAGS = -I/usr/local/include -I/opt/local/include -I../includes -I./
OBJECTS = ioapi.o unzip.o zip.o

all: libminizip
	@$(CP) *.a ../lib

libminizip: $(OBJECTS)
	@$(AR) rs libminizip.a $(OBJECTS)

%.o: %.c
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf *.a