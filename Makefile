EXE=mysh
CC=clang

$(EXE): sh.c .FORCE
	$(CC) -o $(EXE) sh.c

debug: sh.c
	$(CC) -DDEBUG -g -o $(EXE) sh.c

.FORCE:
