.PHONY: atropine physostigmine hyoscine belladonna pilocarpine

all: atropine physostigmine hyoscine belladonna pilocarpine

atropine:
	@make -C atropine

physostigmine:
	@make -C physostigmine

hyoscine:
	@make -C hyoscine

belladonna:
	@make -C belladonna

pilocarpine:
	@make -C pilocarpine


clean:
	@make clean -C atropine
	@make clean -C physostigmine
	@make clean -C hyoscine
	@make clean -C belladonna
	@make clean -C pilocarpine
