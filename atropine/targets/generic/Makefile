export TARGET = GENERIC
export CFLAGS += -DTARGET=$(TARGET) -DBUILD_$(TARGET)=1 -DMODE_$(BUILD_MODE)=1 -mthumb -I. -I./includes
export BUILD_DIR ?= $(shell pwd)/build

LIBS = libc display xnu_patcher
LDFLAGS += -L$(BUILD_DIR) -Tscript.ld
LDLIBS = -ldisplay -lxnu_patcher -lc -lgcc

OBJECTS = main.o printf.o cpu.o jumpto.o boot_args.o kernel.o panic.o
OBJS = $(patsubst %,$(BUILD_DIR)/%,$(OBJECTS))


.PHONY: all clean

all: generic_payload


clean:
	@$(RM) -r $(BUILD_DIR)
	@$(RM) generic_payload

%: $(BASE_DIR)/%/*
	@echo Building $(@)
	@$(MAKE) -C $(BASE_DIR)/$(@)

$(BUILD_DIR)/%.o: %.S
	@echo AS -c $(<) -o $(@)
	@$(AS) $(ASFLAGS) -c $(<) -o $(@)

$(BUILD_DIR)/%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@) 

generic_payload: $(BUILD_DIR) $(LIBS) $(OBJS) $(BUILD_DIR)/entry.o
	@$(CC) -o $(BUILD_DIR)/payload.elf $(LDFLAGS) $(OBJS) $(LDLIBS)
	@$(OBJCOPY) -O binary $(BUILD_DIR)/payload.elf generic_payload


$(BUILD_DIR): 
	@mkdir -p $(BUILD_DIR)


