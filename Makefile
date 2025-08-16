EXE=mysh
CC=clang

$(EXE): sh.c
	$(CC) -o $(EXE) sh.c
