%{
#include "ast.h"
#include "parser.tab.h"
#include <stdlib.h>
#include <string.h>
%}
%option noyywrap yylineno
DIGIT [0-9]
%%
[ \t\r\n]+                 ;
"//".*                     ;
"print"                    { return PRINT; }
{DIGIT}+(\.{DIGIT}+)?([eE][+-]?{DIGIT}+)?  { yylval.num = atof(yytext); return NUM; }
[A-Za-z_][A-Za-z0-9_]*     { yylval.id = strdup(yytext); return ID; }
[-+*/%^=();]               { return yytext[0]; }
.                          { fprintf(stderr, "Лексична помилка (рядок %d): '%s'\n", yylineno, yytext); exit(1); }
%%
