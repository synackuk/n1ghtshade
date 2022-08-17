CC = gcc
AR = ar
RM = rm
CFLAGS = -arch x86_64 -arch arm64 -I/usr/include -I../static/include -I. -I./idevicerestore -O3 -Wall -DIDEVICERESTORE_NOMAIN=1 -DHAVE_REALPATH=1 -DPACKAGE_VERSION=\"1\"
OBJECTS = libbelladonna.o recovery.o dfu.o ramdisk.o device.o exploits/checkm8/checkm8.o idevicerestore/idevicerestore.o idevicerestore/common.o idevicerestore/tss.o idevicerestore/fls.o idevicerestore/mbn.o idevicerestore/img3.o idevicerestore/img4.o idevicerestore/ftab.o idevicerestore/ipsw.o idevicerestore/normal.o idevicerestore/dfu.o idevicerestore/recovery.o idevicerestore/restore.o idevicerestore/asr.o idevicerestore/fdr.o idevicerestore/limera1n.o idevicerestore/download.o idevicerestore/locking.o idevicerestore/sha512.o



all: checkm8_payload libbelladonna.a belladonna

checkm8_payload:
	@$(MAKE) -C exploits/checkm8/payload

belladonna: libbelladonna.a
	@echo Building $(@)
	@$(MAKE) -C tools

libbelladonna.a: $(PAYLOADS) $(OBJECTS)
	@echo Building $(@)
	@echo AR rs $(@) $(OBJECTS)
	@$(AR) rs $(@) $(OBJECTS)

%.o: %.c
	@echo CC -c $(<) -o $(@)
	@$(CC) $(CFLAGS) -c $(<) -o $(@)


clean:
	@$(RM) -rf *.o
	@$(RM) -rf *.a
	@$(RM) -rf exploits/checkm8/*.o
	@$(RM) -rf idevicerestore/*.o
	@$(MAKE) clean -C tools
	@$(MAKE) clean -C exploits/checkm8/payload