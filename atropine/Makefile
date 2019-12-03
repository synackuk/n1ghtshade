.PHONY: plib iBoot32Patcher targets
all: plib iBoot32Patcher targets

plib:
	@echo Building $(@)
	@make -C plib

iBoot32Patcher:
	@echo Building $(@)
	@make -C iBoot32Patcher

targets:
	@echo Building $(@)
	@make -C targets

clean:
	@make clean -C plib
	@make clean -C iBoot32Patcher
	@make clean -C targets