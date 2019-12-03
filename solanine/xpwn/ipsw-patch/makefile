CC = gcc
AR = ar
RM = rm
CP = cp
CFLAGS = -I../minizip -I/usr/local/include -I/usr/local/Cellar/openssl/1.0.2t/include -I/opt/local/include -I../includes -I./
OBJECTS = 8900.o bspatch.o ibootim.o img2.o img3.o libxpwn.o lzss.o lzssfile.o nor_files.o outputstate.o plist.o pwnutil.o ticket.o validate.o

all: libxpwn.a
	@$(CP) *.a ../lib

libxpwn.a: $(OBJECTS)
	@echo Building $(@)
	@echo AR rs $(@) $(OBJECTS)
	@$(AR) rs $(@) $(OBJECTS)

%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

clean:
	@$(RM) -rf *.o
	@$(RM) -rf *.a