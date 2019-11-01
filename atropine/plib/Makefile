CROSS = arm-elf-
CC = $(CROSS)gcc-4.7
AR = $(CROSS)ar
RM = rm
BASE_DIR = ./
BUILD_DIR = bin
OBJECTS = atoi.o div.o memcmp.o memmem.o memmove.o memset.o strcmp.o strcpy.o strncat.o strcat.o strlen.o xtoi.o
OBJS = $(patsubst %,$(BUILD_DIR)/%,$(OBJECTS))
CFLAGS = -Wall -W -pedantic -Wno-long-long -Wno-main -Os -fcall-used-r9 -fpie -I../

$(BUILD_DIR)/%.o: $(BASE_DIR)%.c
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

libp.a: $(OBJS)
	@$(AR) crus libp.a $(OBJS)
all: libp.a

clean:
	@$(RM) -rf $(BUILD_DIR)
	@$(RM) -rf libp.a