EXE=mysh
SRC = $(wildcard src/*.c)

OUTPATH = bin/
OUTFILE = $(OUTPATH)$(EXE)
OBJPATH = obj/
OBJ = $(addprefix $(OBJPATH), $(notdir $(SRC:.c=.o)))

CC=clang
CCFLAGS=

INCLUDE_FLAGS := -I./include

all: $(OUTFILE)

debug: CCFLAGS += -DDEBUG -g
debug: $(OUTFILE)


$(OUTFILE): dir $(OBJ)
	$(CC) $(CCFLAGS) $(INCLUDE_FLAGS) $(OBJ) -o $(OUTFILE)

$(OBJPATH)%.o: src/%.c
	$(CC) $(CCFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

dir:
	mkdir -p $(OUTPATH)
	mkdir -p $(OBJPATH)

clean:
	rm -rf $(OUTPATH)
	rm -rf $(OBJPATH)

.PHONY: all
