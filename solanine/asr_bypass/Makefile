CD = cd
TAR = tar
RM = rm
BIN2C = bin2c

all:
	@make copy -C asr
	@make copy -C asr_bypass
	@$(CD) ramdisk_tar && $(TAR) -cf ../ramdisk_patch.tar .
	@$(BIN2C) ramdisk_patch.tar ../asr_bypass.h asr_patch_tar
clean:
	@make clean -C asr
	@make clean -C asr_bypass
	@$(RM) -rf ramdisk_tar/usr/lib/asr_bypass.dylib
	@$(RM) -rf ramdisk_tar/usr/sbin/asr
	@$(RM) -rf ramdisk_patch.tar
	@$(RM) -rf ../asr_bypass.h