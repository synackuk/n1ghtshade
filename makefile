all:
	@make -C atropine
	@make -C physostigmine
	@make -C hyoscine
	@make -C belladonna
	@make -C solanine
clean:
	@make clean -C atropine
	@make clean -C physostigmine
	@make clean -C hyoscine
	@make clean -C belladonna
	@make clean -C solanine
