CROSS = arm-elf-
CC = $(CROSS)gcc-4.7
RM = rm
MKDIR = mkdir
BIN2C = bin2c
BASE_DIR = ../../
BUILD_DIR = bin
CFLAGS =  -nostdlib -Wno-multichar -Wno-packed-bitfield-compat -Os -mlittle-endian -std=gnu99 -fpie -I$(BASE_DIR)/includes -I$(BASE_DIR)/plib -I$(BASE_DIR)/iBoot32Patcher -I.
LDFLAGS = -L$(BASE_DIR)plib -L$(BASE_DIR)iBoot32Patcher -nostdlib -Tscript.ld
LDLIBS = -liBoot32Patcher -lp -lgcc
OBJECTS = memory.o finders.o kernel.o relocate.o image.o command.o menu_commands.o framebuffer.o constants.o main.o
OBJS = $(patsubst %,$(BUILD_DIR)/%,$(OBJECTS))
OBJCOPY = $(CROSS)objcopy

$(BUILD_DIR)/%.o: $(BASE_DIR)%.S
	@echo CC -c $(<) -o $(@)
	@$(MKDIR) -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $(<) -o $(@) 

$(BUILD_DIR)/%.o: $(BASE_DIR)%.c
	@echo CC -c $(<) -o $(@)
	@$(MKDIR) -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $(<) -o $(@) 

atropine: $(OBJS) | bin/entry.o
	@$(CC) -o $(BUILD_DIR)/payload.elf $(LDFLAGS) $(OBJS) $(LDLIBS)
	@$(OBJCOPY) -O binary $(BUILD_DIR)/payload.elf atropine
	@-$(MKDIR) ../../../belladonna/payloads
	@$(BIN2C) atropine ../../../belladonna/payloads/atropine.h atropine
	
all: atropine

clean:
	@$(RM) -rf $(BUILD_DIR)
	@$(RM) -rf atropine
	@$(RM) -rf ../../../belladonna/payloads/atropine.h