all:
	@make -C atropine
	@make -C belladonna
	@make -C solanine
clean:
	@make clean -C atropine
	@make clean -C belladonna
	@make clean -C solanine
