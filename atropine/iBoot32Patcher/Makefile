CROSS = arm-elf-
CC = $(CROSS)gcc-4.7
AR = $(CROSS)ar
RM = rm
BASE_DIR = ./
BUILD_DIR = bin
OBJECTS = finders.o functions.o iBoot32Patcher.o patchers.o
OBJS = $(patsubst %,$(BUILD_DIR)/%,$(OBJECTS))
CFLAGS = -Wall -W -std=gnu99 -Wno-multichar -Wno-packed-bitfield-compat -Wno-long-long -Wno-main -Os -fcall-used-r9 -fpie -I. -I../targets/iBEC -I../includes 

$(BUILD_DIR)/%.o: $(BASE_DIR)%.c
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

libiBoot32Patcher.a: $(OBJS)
	@$(AR) crus libiBoot32Patcher.a $(OBJS)
all: libiBoot32Patcher.a

clean:
	@$(RM) -rf $(BUILD_DIR)
	@$(RM) -rf libiBoot32Patcher.a