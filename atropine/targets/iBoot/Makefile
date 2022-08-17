export TARGET = IBOOT
export CFLAGS += -DTARGET=$(TARGET) -DBUILD_$(TARGET)=1 -DMODE_$(BUILD_MODE)=1 -mthumb -I. -I./includes
export BUILD_DIR ?= $(shell pwd)/build

LIBS = libc display iBoot_Patcher
LDFLAGS += -L$(BUILD_DIR) -Tscript.ld
LDLIBS = -ldisplay -liBoot_Patcher -lc -lgcc

OBJECTS = main.o address.o jumpto_hook.o handler.o command.o menu_commands.o
OBJS = $(patsubst %,$(BUILD_DIR)/%,$(OBJECTS))


.PHONY: all clean

all: iBoot_payload


clean:
	@$(RM) -r $(BUILD_DIR)
	@$(RM) iBoot_payload

%: $(BASE_DIR)/%/*
	@echo Building $(@)
	@$(MAKE) -C $(BASE_DIR)/$(@)

$(BUILD_DIR)/%.o: %.S
	@echo AS -c $(<) -o $(@)
	@$(AS) $(ASFLAGS) -c $(<) -o $(@)

$(BUILD_DIR)/%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@) 

iBoot_payload: $(BUILD_DIR) $(BUILD_DIR)/generic_payload.h $(LIBS) $(OBJS) $(BUILD_DIR)/entry.o
	@$(CC) -o $(BUILD_DIR)/payload.elf $(LDFLAGS) $(OBJS) $(LDLIBS)
	@$(OBJCOPY) -O binary $(BUILD_DIR)/payload.elf iBoot_payload

$(BUILD_DIR)/generic_payload.h:
	@$(BIN2C) $(BASE_DIR)/targets/generic/generic_payload $(BUILD_DIR)/generic_payload.h generic_payload

$(BUILD_DIR): 
	@mkdir -p $(BUILD_DIR)


