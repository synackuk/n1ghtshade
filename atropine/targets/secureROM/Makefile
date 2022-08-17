export TARGET = SECUREROM
export CFLAGS += -DTARGET=$(TARGET) -DBUILD_$(TARGET)=1 -mthumb
export BUILD_DIR ?= $(shell pwd)/build

LIBS = iBoot_Patcher libc
LDFLAGS += -L$(BUILD_DIR) 
LDLIBS = -liBoot_Patcher -lc

OBJECTS = entry.o
OBJS = $(patsubst %,$(BUILD_DIR)/%,$(OBJECTS))


.PHONY: all clean

all: secureROM_payload

clean:
	@$(RM) -r $(BUILD_DIR)
	@$(RM) secureROM_payload

%: $(BASE_DIR)/%/*
	@echo Building $(@)
	@$(MAKE) -C $(BASE_DIR)/$(@)

$(BUILD_DIR)/%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@) 

secureROM_payload: $(BUILD_DIR) $(LIBS) $(OBJS)
	@$(CC) -o $(BUILD_DIR)/payload.elf $(LDFLAGS) $(OBJS) $(LDLIBS)
	@$(OBJCOPY) -O binary $(BUILD_DIR)/payload.elf secureROM_payload


$(BUILD_DIR): 
	@mkdir -p $(BUILD_DIR)


