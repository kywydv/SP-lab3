#ifndef AST_H
#define AST_H
#include <stdio.h>

typedef enum { N_PROG, N_NUM, N_VAR, N_BIN, N_NEG, N_ASSIGN, N_PRINT } NodeType;

typedef struct Node {
    NodeType type;
    char op;              /* + - * / % ^ */
    double val;           /* N_NUM */
    char *name;           /* N_VAR, N_ASSIGN */
    struct Node *l, *r;   /* operands / child */
    struct Node *next;    /* next statement */
} Node;

Node *mk_num(double v);
Node *mk_var(char *name);
Node *mk_bin(char op, Node *l, Node *r);
Node *mk_neg(Node *e);
Node *mk_assign(char *name, Node *e);
Node *mk_print(Node *e);
Node *mk_prog(Node *stmts);
Node *append_stmt(Node *list, Node *s);

void run_eval(Node *prog);              /* обчислення AST */
void gen_c(Node *prog, FILE *out);      /* генерація коду на C */
void dump_json(Node *n, FILE *out);     /* AST -> JSON для d3.js */
#endif
