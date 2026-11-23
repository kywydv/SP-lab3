%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
int yylex(void);
void yyerror(const char *s);
extern int yylineno;
Node *root;
%}
%union { double num; char *id; Node *node; }
%token <num> NUM
%token <id> ID
%token PRINT
%type <node> program stmts stmt expr

%left '+' '-'
%left '*' '/' '%'
%right UMINUS
%right '^'

%%
program : stmts              { root = mk_prog($1); }
        ;
stmts   : stmt               { $$ = $1; }
        | stmts stmt         { $$ = append_stmt($1, $2); }
        ;
stmt    : ID '=' expr ';'    { $$ = mk_assign($1, $3); }
        | PRINT expr ';'     { $$ = mk_print($2); }
        ;
expr    : expr '+' expr      { $$ = mk_bin('+', $1, $3); }
        | expr '-' expr      { $$ = mk_bin('-', $1, $3); }
        | expr '*' expr      { $$ = mk_bin('*', $1, $3); }
        | expr '/' expr      { $$ = mk_bin('/', $1, $3); }
        | expr '%' expr      { $$ = mk_bin('%', $1, $3); }
        | expr '^' expr      { $$ = mk_bin('^', $1, $3); }
        | '-' expr %prec UMINUS { $$ = mk_neg($2); }
        | '(' expr ')'       { $$ = $2; }
        | NUM                { $$ = mk_num($1); }
        | ID                 { $$ = mk_var($1); }
        ;
%%
void yyerror(const char *s) {
    fprintf(stderr, "Синтаксична помилка (рядок %d): %s\n", yylineno, s);
}
