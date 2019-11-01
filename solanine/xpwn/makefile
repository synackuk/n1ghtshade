MKDIR = mkdir
RM = rm
all:
	@$(RM) -rf lib
	@$(MKDIR) lib
	@make -C common
	@make -C dmg
	@make -C hfs
	@make -C ipsw-patch
	@make -C minizip
clean:
	@$(RM) -rf lib
	@make clean -C common
	@make clean -C dmg
	@make clean -C hfs
	@make clean -C ipsw-patch
	@make clean -C minizip