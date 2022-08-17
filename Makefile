
.PHONY: all atropine belladonna hyoscine physostigmine

all: atropine physostigmine hyoscine belladonna pilocarpine

pilocarpine: belladonna
	@$(MAKE) -C pilocarpine

atropine:
	@$(MAKE) -C atropine
	@mkdir -p belladonna/payloads
	@bin2c atropine/targets/secureROM/secureROM_payload belladonna/payloads/atropine_securerom_payload.h atropine_securerom_payload
	@bin2c atropine/targets/iBEC/iBEC_payload belladonna/payloads/atropine_ibec_payload.h atropine_ibec_payload
	@bin2c atropine/targets/iBoot/iBoot_payload belladonna/payloads/atropine_iboot_payload.h atropine_iboot_payload

physostigmine:
	@$(MAKE) -C physostigmine

hyoscine: physostigmine
	@$(MAKE) -C hyoscine
	@mkdir -p belladonna/payloads
	@bin2c hyoscine/ramdisk.dmg belladonna/payloads/hyoscine_ramdisk.h hyoscine_ramdisk

belladonna: atropine hyoscine
	@$(MAKE) -C belladonna

clean:
	@$(MAKE) clean -C atropine
	@$(MAKE) clean -C hyoscine
	@$(MAKE) clean -C physostigmine
	@$(RM) -rf belladonna/payloads
	@$(MAKE) clean -C belladonna
	@$(MAKE) clean -C pilocarpine