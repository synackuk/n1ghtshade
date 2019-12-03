CC = gcc
AR = ar
RM = rm
CP = cp
CFLAGS = -I/usr/local/include -I/opt/local/include -I../includes
OBJECTS = checksum.o dmg.o dmgfile.o dmglib.o filevault.o io.o filevault.o partition.o resources.o udif.o

all: libdmg.a
	@$(CP) *.a ../lib

libdmg.a: $(OBJECTS)
	@echo Building $(@)
	@echo AR rs $(@) $(OBJECTS)
	@$(AR) rs $(@) $(OBJECTS)

%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf *.a