CC = gcc
AR = ar
RM = rm
CP = cp
CFLAGS = -I/usr/local/include -I/opt/local/include -I../includes
OBJECTS = btree.o catalog.o extents.o fastunicodecompare.o flatfile.o hfs.o hfscompress.o hfslib.o rawfile.o utility.o volume.o xattr.o

all: libhfs
	@$(CP) *.a ../lib

libhfs: $(OBJECTS)
	@$(AR) rs libhfs.a $(OBJECTS)

%.o: %.c
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf *.a