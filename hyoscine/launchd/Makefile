RM = rm
CC = `xcrun -find cc`
STRIP = `xcrun -find strip`
LDID = ldid
SYSROOT = $(THEOS)/sdks/iPhoneOS6.0.sdk
OBJECTS = syscalls.o launchd.o common.o install.o modules/kill_software_update.o modules/fstab.o modules/n1ghtshade_app.o modules/non_default_apps.o  modules/hacktivate.o modules/jb_init.o
CFLAGS = -I. -arch armv6 -static -nodefaultlibs -nostdlib -isysroot$(SYSROOT) -I$(SYSROOT) -fno-stack-protector
LDFLAGS = -arch armv6 -Wl,-e,_main -Wl,-no_pie -nostartfiles

all: launchd
	@echo STRIP launchd
	@$(STRIP) launchd
	@$(LDID) -S launchd

clean:
	@$(RM) -rf *.o
	@$(RM) -rf modules/*.o
	@$(RM) -rf launchd

launchd: $(OBJECTS)
	@echo CC -c $(<) -o $(@)
	@$(CC) -o launchd $(OBJECTS) $(CFLAGS) $(LDFLAGS)

%.o: %.c
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

%.o: %.S
	@$(CC) $(CFLAGS) -c $(<) -o $(@)

