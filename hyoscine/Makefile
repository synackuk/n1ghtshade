BIN2C = bin2c
IMAGE3MAKER = image3maker
HFSPLUS = hfsplus
RM = rm
CP = cp
MKDIR = mkdir

all: hyoscine.img3
	@-$(MKDIR) ../belladonna/payloads/
	@$(BIN2C) hyoscine.img3 ../belladonna/payloads/hyoscine.h hyoscine

hyoscine.img3: hyoscine.dmg
	@$(IMAGE3MAKER) -t rdsk -f hyoscine.dmg -o hyoscine.img3

hyoscine.dmg: launchd/launchd
	@$(CP) template.dmg hyoscine.dmg
	@$(HFSPLUS) hyoscine.dmg add launchd/launchd /sbin/launchd
	@$(HFSPLUS) hyoscine.dmg chmod 755 /sbin/launchd
	@$(HFSPLUS) hyoscine.dmg addall modules /modules

launchd/launchd:
	@make -C launchd


clean:
	@make clean -C launchd
	@$(RM) -rf modules/*
	@$(RM) -rf hyoscine.dmg
	@$(RM) -rf hyoscine.img3