.PHONY: kill_software_update fstab n1ghtshade_app non_default_apps jb_init hacktivate

all: kill_software_update fstab n1ghtshade_app non_default_apps jb_init hacktivate

kill_software_update:
	@echo Building $(@)
	@make copy -C $(@)

fstab:
	@echo Building $(@)
	@make copy -C $(@)

n1ghtshade_app:
	@echo Building $(@)
	@make copy -C $(@)

non_default_apps:
	@echo Building $(@)
	@make copy -C $(@)

jb_init:
	@echo Building $(@)
	@make copy -C $(@)

hacktivate:
	@echo Building $(@)
	@make -C $(@)

clean:
	@make clean -C kill_software_update
	@make clean -C fstab
	@make clean -C n1ghtshade_app
	@make clean -C non_default_apps
	@make clean -C jb_init
	@make clean -C hacktivate
