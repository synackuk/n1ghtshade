CC = gcc
AR = ar
RM = rm
CP = cp
CFLAGS = -I/usr/local/include -I/opt/local/include -I../includes -I./
OBJECTS = ioapi.o unzip.o zip.o

all: libminizip.a
	@$(CP) *.a ../lib

libminizip.a: $(OBJECTS)
	@echo Building $(@)
	@echo AR rs $(@) $(OBJECTS)
	@$(AR) rs $(@) $(OBJECTS)

%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf *.a