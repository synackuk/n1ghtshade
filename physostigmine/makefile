.PHONY: jb_init n1ghtshade_app non_default_apps uicache sysupdate jailbreak_tar hacktivate

all: jb_init n1ghtshade_app non_default_apps uicache sysupdate hacktivate
	@cp non_default_apps/non_default_apps ../hyoscine/files
	@cp uicache/uicache ../hyoscine/files
	@cp sysupdate/sysupdate ../hyoscine/files
	@tar -cvf ../hyoscine/files/n1ghtshade.tar -C n1ghtshade_app/ n1ghtshade.app
	@cp jb_init/jb_init ../hyoscine/files
	@cp hacktivate/hacktivate_dylib/hacktivate.dylib ../hyoscine/files
	@cp hacktivate/hacktivate_installer/hacktivate_installer ../hyoscine/files
	@cp hacktivate/lockdownd/lockdownd ../hyoscine/files


jb_init:
	@echo Building $(@)
	@make copy -C $(@)

non_default_apps:
	@echo Building $(@)
	@make copy -C $(@)

uicache:
	@echo Building $(@)
	@make copy -C $(@)

sysupdate:
	@echo Building $(@)
	@make copy -C $(@)

n1ghtshade_app:
	@echo Building $(@)
	@make copy -C $(@)

hacktivate:
	@echo Building $(@)
	@make -C $(@)

clean:
	@make clean -C n1ghtshade_app
	@make clean -C jb_init
	@make clean -C non_default_apps
	@make clean -C uicache
	@make clean -C sysupdate
	@make clean -C hacktivate