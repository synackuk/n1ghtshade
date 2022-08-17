CFLAGS += -I. -I$(BUILD_DIR)
ASFLAGS += -mthumb
OBJECTS = functions.o finders.o patchers.o xnu32patcher.o
OBJS = $(patsubst %,$(BUILD_DIR)/%,$(OBJECTS))

.PHONY: all

$(BUILD_DIR)/%.o: %.S
	@echo AS -c $(<) -o $(@)
	@$(AS) $(ASFLAGS) -c $(<) -o $(@)

$(BUILD_DIR)/%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@) 

$(BUILD_DIR)/libxnu_patcher.a: $(BUILD_DIR)/i_can_has_debugger_patch.h | $(OBJS)
	@echo AR crus $(@) $(OBJECTS)
	@$(AR) crus $(@) $(OBJS)

all: $(BUILD_DIR)/libxnu_patcher.a


$(BUILD_DIR)/i_can_has_debugger_patch.h: $(BUILD_DIR)/i_can_has_debugger_patch.o
	@$(OBJCOPY) -O binary $(BUILD_DIR)/i_can_has_debugger_patch.o $(BUILD_DIR)/i_can_has_debugger_patch
	@$(BIN2C) $(BUILD_DIR)/i_can_has_debugger_patch $(BUILD_DIR)/i_can_has_debugger_patch.h i_can_has_debugger_patch