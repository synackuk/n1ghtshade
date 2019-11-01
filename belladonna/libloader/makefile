CC = gcc
AR = ar
RM = rm
CFLAGS = -I/usr/local/include -I/opt/local/include -I.
OBJECTS = libloader.o

all: libloader.a

libloader.a: $(OBJECTS)
	@$(AR) rs libloader.a $(OBJECTS)

%.o: %.c
	@$(CC) $(CFLAGS) -c $(<) -o $(@)
	
clean:
	@$(RM) -rf *.o
	@$(RM) -rf *.a