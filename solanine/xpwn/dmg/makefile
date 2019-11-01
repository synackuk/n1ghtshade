CC = gcc
AR = ar
RM = rm
CP = cp
CFLAGS = -I/usr/local/include -I/opt/local/include -I../includes
OBJECTS = checksum.o dmg.o dmgfile.o dmglib.o filevault.o io.o filevault.o partition.o resources.o udif.o

all: libdmg
	@$(CP) *.a ../lib

libdmg: $(OBJECTS)
	@$(AR) rs libdmg.a $(OBJECTS)

%.o: %.c
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf *.a