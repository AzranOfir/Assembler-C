assembler: assembler.c commands.c first_pass.c labelTable.c macro.c parser.c second_pass.c utils.c commands.h first_pass.h labelTable.h macro.h parser.h second_pass.h utils.h

	gcc -Wall -ansi -pedantic assembler.c commands.c first_pass.c labelTable.c macro.c parser.c second_pass.c utils.c -o assembler
