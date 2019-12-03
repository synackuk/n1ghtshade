CC = gcc
AR = ar
RM = rm
CP = cp
CFLAGS = -I/usr/local/include -I/opt/local/include -I../includes
OBJECTS = btree.o catalog.o extents.o fastunicodecompare.o flatfile.o hfs.o hfscompress.o hfslib.o rawfile.o utility.o volume.o xattr.o

all: libhfs.a
	@$(CP) *.a ../lib

libhfs.a: $(OBJECTS)
	@echo Building $(@)
	@echo AR rs $(@) $(OBJECTS)
	@$(AR) rs $(@) $(OBJECTS)

%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf *.a