CC = gcc
AR = ar
RM = rm
CP = cp
CFLAGS = -I/usr/local/include -I/opt/local/include -I../includes
OBJECTS = abstractfile.o base64.o

all: libcommon
	@$(CP) *.a ../lib

libcommon: $(OBJECTS)
	@$(AR) rs libcommon.a $(OBJECTS)

%.o: %.c
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf *.a