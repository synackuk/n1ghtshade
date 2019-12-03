MKDIR = mkdir
RM = rm

.PHONY: lib common dmg hfs ipsw-patch minizip

all: lib common dmg hfs ipsw-patch minizip

lib:
	$(MKDIR) lib

common:
	@echo Building $(@)
	@make -C $(@)

dmg:
	@echo Building $(@)
	@make -C $(@)

hfs:
	@echo Building $(@)
	@make -C $(@)

ipsw-patch:
	@echo Building $(@)
	@make -C $(@)

minizip:
	@echo Building $(@)
	@make -C $(@)


clean:
	@$(RM) -rf lib
	@make clean -C common
	@make clean -C dmg
	@make clean -C hfs
	@make clean -C ipsw-patch
	@make clean -C minizip