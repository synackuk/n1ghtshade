CC = gcc
AR = ar
RM = rm
CP = cp
CFLAGS = -I/usr/local/include -I/opt/local/include -I../includes
OBJECTS = abstractfile.o base64.o

all: libcommon.a
	@$(CP) *.a ../lib

libcommon.a: $(OBJECTS)
	@echo Building $(@)
	@echo AR rs $(@) $(OBJECTS)
	@$(AR) rs $(@) $(OBJECTS)

%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf *.a