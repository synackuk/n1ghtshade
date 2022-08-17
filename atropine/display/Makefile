CFLAGS += -I.
OBJECTS = text.o shape.o display.o image.o
OBJS = $(patsubst %,$(BUILD_DIR)/%,$(OBJECTS))

.PHONY: all

$(BUILD_DIR)/%.o: %.S
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@) 

$(BUILD_DIR)/%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@) 

$(BUILD_DIR)/libdisplay.a: $(OBJS)
	@echo AR crus $(@) $(OBJECTS)
	@$(AR) crus $(@) $(OBJS)

all: $(BUILD_DIR)/libdisplay.a