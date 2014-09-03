all: shell

shell:	lex.yy.o shell.o
	cc -o shell lex.yy.o shell.o
	rm shell.o lex.yy.o lex.yy.c

lex.yy.c: lex.c
	flex lex.c

lex.yy.o: lex.yy.c
	cc -c lex.yy.c

shell.o: shell.c
	cc -c shell.c




