CFLAGS += -I.
OBJECTS = atoi.o memcpy.o memmem.o strlen.o memcmp.o strcmp.o strcat.o memset.o clear_icache.o putchar.o

OBJS = $(patsubst %,$(BUILD_DIR)/%,$(OBJECTS))

.PHONY: all

$(BUILD_DIR)/%.o: %.S
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@) 

$(BUILD_DIR)/%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@) 

$(BUILD_DIR)/libc.a: $(OBJS)
	@echo AR crus $(@) $(OBJECTS)
	@$(AR) crus $(@) $(OBJS)

all: $(BUILD_DIR)/libc.a