calc: parser.tab.c lex.yy.c ast.c main.c
	gcc -o calc parser.tab.c lex.yy.c ast.c main.c -lm
parser.tab.c parser.tab.h: parser.y
	bison -d -Wcounterexamples parser.y
lex.yy.c: lexer.l parser.tab.h
	flex lexer.l
clean:
	rm -f calc parser.tab.* lex.yy.c out.c ast.json
