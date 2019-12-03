.PHONY: hacktivate_dylib lockdownd

all: hacktivate_dylib lockdownd

hacktivate_dylib:
	@echo Building $(@)
	@make copy -C $(@)

lockdownd:
	@echo Building $(@)
	@make copy -C $(@)

clean:
	@make clean -C hacktivate_dylib
	@make clean -C lockdownd