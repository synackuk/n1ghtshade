.PHONY: hacktivate_dylib lockdownd hacktivate_installer

all: hacktivate_dylib lockdownd hacktivate_installer

hacktivate_dylib:
	@echo Building $(@)
	@make copy -C $(@)

lockdownd:
	@echo Building $(@)
	@make copy -C $(@)

hacktivate_installer:
	@echo Building $(@)
	@make copy -C $(@)

clean:
	@make clean -C hacktivate_dylib
	@make clean -C lockdownd
	@make clean -C hacktivate_installer