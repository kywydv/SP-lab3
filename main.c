#include <stdio.h>
#include <string.h>
#include "ast.h"
extern FILE *yyin;
extern Node *root;
int yyparse(void);

/* Використання: calc input.txt [-e] [-c out.c] [-j ast.json] */
int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "Використання: %s input.txt [-e] [-c out.c] [-j ast.json]\n", argv[0]); return 1; }
    yyin = fopen(argv[1], "r");
    if (!yyin) { perror("input"); return 1; }
    if (yyparse() != 0) return 1;
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "-e")) run_eval(root);
        else if (!strcmp(argv[i], "-c") && i + 1 < argc) { FILE *f = fopen(argv[++i], "w"); gen_c(root, f); fclose(f); }
        else if (!strcmp(argv[i], "-j") && i + 1 < argc) { FILE *f = fopen(argv[++i], "w"); dump_json(root, f); fclose(f); }
    }
    return 0;
}
