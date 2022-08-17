
.PHONY: all launchd jailbreakd

all: launchd jailbreakd
	@cp template.dmg ramdisk.dmg
	@hfsplus ramdisk.dmg add jailbreakd/jailbreakd /jailbreakd
	@hfsplus ramdisk.dmg add launchd/launchd /sbin/launchd
	@hfsplus ramdisk.dmg chmod 755 /jailbreakd
	@hfsplus ramdisk.dmg chmod 755 /sbin/launchd
	@hfsplus ramdisk.dmg addall files/ /files
	@hfsplus ramdisk.dmg chmod 755 /files/non_default_apps
	@hfsplus ramdisk.dmg chmod 755 /files/sysupdate
	@hfsplus ramdisk.dmg chmod 755 /files/uicache
	@hfsplus ramdisk.dmg chmod 755 /files/hacktivate_installer
	@hfsplus ramdisk.dmg chmod 755 /files/jb_init
	@hfsplus ramdisk.dmg chmod 755 /files/lockdownd
	@hfsplus ramdisk.dmg chmod 755 /files/hacktivate.dylib

launchd:
	@$(MAKE) copy -C launchd

jailbreakd:
	@$(MAKE) copy -C jailbreakd

clean:
	@$(MAKE) clean -C launchd
	@$(MAKE) clean -C jailbreakd
	@$(RM) -rf ramdisk.dmg
	@$(RM) -rf files/*
	@touch files/.placeholder