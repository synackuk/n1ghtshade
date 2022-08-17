CFLAGS += -I. -I$(BUILD_DIR)
ASFLAGS += -mthumb
OBJECTS = functions.o finders.o patchers.o iBoot32Patcher.o
OBJS = $(patsubst %,$(BUILD_DIR)/%,$(OBJECTS))

.PHONY: all

$(BUILD_DIR)/%.o: %.S
	@echo AS -c $(<) -o $(@)
	@$(AS) $(ASFLAGS) -c $(<) -o $(@)

$(BUILD_DIR)/%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@) 

$(BUILD_DIR)/libiBoot_Patcher.a: $(BUILD_DIR)/trampoline.h | $(OBJS)
	@echo AR crus $(@) $(OBJECTS)
	@$(AR) crus $(@) $(OBJS)

all: $(BUILD_DIR)/libiBoot_Patcher.a

$(BUILD_DIR)/trampoline.h: $(BUILD_DIR)/trampoline.o
	@$(OBJCOPY) -O binary $(BUILD_DIR)/trampoline.o $(BUILD_DIR)/trampoline
	@$(BIN2C) $(BUILD_DIR)/trampoline $(BUILD_DIR)/trampoline.h trampoline
