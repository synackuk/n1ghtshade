.PHONY: atropine physostigmine hyoscine belladonna solanine pilocarpine

all: atropine physostigmine hyoscine belladonna solanine pilocarpine

atropine:
	@make -C atropine

physostigmine:
	@make -C physostigmine

hyoscine:
	@make -C hyoscine

belladonna:
	@make -C belladonna

solanine:
	@make -C solanine

pilocarpine:
	@make -C pilocarpine


clean:
	@make clean -C atropine
	@make clean -C physostigmine
	@make clean -C hyoscine
	@make clean -C belladonna
	@make clean -C solanine
	@make clean -C pilocarpine
