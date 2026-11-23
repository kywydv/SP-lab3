#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ast.h"

static Node *mk(NodeType t) {
    Node *n = calloc(1, sizeof(Node));
    n->type = t;
    return n;
}
Node *mk_num(double v)           { Node *n = mk(N_NUM); n->val = v; return n; }
Node *mk_var(char *s)            { Node *n = mk(N_VAR); n->name = s; return n; }
Node *mk_bin(char op, Node *l, Node *r) { Node *n = mk(N_BIN); n->op = op; n->l = l; n->r = r; return n; }
Node *mk_neg(Node *e)            { Node *n = mk(N_NEG); n->l = e; return n; }
Node *mk_assign(char *s, Node *e){ Node *n = mk(N_ASSIGN); n->name = s; n->l = e; return n; }
Node *mk_print(Node *e)          { Node *n = mk(N_PRINT); n->l = e; return n; }
Node *mk_prog(Node *s)           { Node *n = mk(N_PROG); n->l = s; return n; }
Node *append_stmt(Node *list, Node *s) {
    if (!list) return s;
    Node *p = list;
    while (p->next) p = p->next;
    p->next = s;
    return list;
}

/* ---------- Семантика: обчислення ---------- */
#define MAXVARS 256
static char *vn[MAXVARS];
static double vv[MAXVARS];
static int nv = 0;

static int find_var(const char *name, int create) {
    for (int i = 0; i < nv; i++) if (!strcmp(vn[i], name)) return i;
    if (!create) return -1;
    vn[nv] = strdup(name);
    vv[nv] = 0;
    return nv++;
}

static double eval(Node *n) {
    switch (n->type) {
    case N_NUM: return n->val;
    case N_VAR: {
        int i = find_var(n->name, 0);
        if (i < 0) { fprintf(stderr, "Помилка: змінна '%s' не визначена\n", n->name); exit(2); }
        return vv[i];
    }
    case N_NEG: return -eval(n->l);
    case N_BIN: {
        double a = eval(n->l), b = eval(n->r);
        switch (n->op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0) { fprintf(stderr, "Помилка: ділення на нуль\n"); exit(2); }
            return a / b;
        case '%': return fmod(a, b);
        case '^': return pow(a, b);
        }
    }
    default: return 0;
    }
}

void run_eval(Node *prog) {
    for (Node *s = prog->l; s; s = s->next) {
        if (s->type == N_ASSIGN) vv[find_var(s->name, 1)] = eval(s->l);
        else if (s->type == N_PRINT) printf("%g\n", eval(s->l));
    }
}

/* ---------- Генерація коду на C ---------- */
static void gen_expr(Node *n, FILE *o) {
    switch (n->type) {
    case N_NUM: fprintf(o, "%.17g", n->val); break;
    case N_VAR: fprintf(o, "%s", n->name); break;
    case N_NEG: fprintf(o, "(-"); gen_expr(n->l, o); fprintf(o, ")"); break;
    case N_BIN:
        if (n->op == '^' || n->op == '%') {
            fprintf(o, n->op == '^' ? "pow(" : "fmod(");
            gen_expr(n->l, o); fprintf(o, ", "); gen_expr(n->r, o); fprintf(o, ")");
        } else {
            fprintf(o, "("); gen_expr(n->l, o);
            fprintf(o, " %c ", n->op);
            gen_expr(n->r, o); fprintf(o, ")");
        }
        break;
    default: break;
    }
}

void gen_c(Node *prog, FILE *o) {
    fprintf(o, "/* Згенеровано транслятором */\n#include <stdio.h>\n#include <math.h>\n\nint main(void) {\n");
    char *seen[MAXVARS]; int ns = 0;
    for (Node *s = prog->l; s; s = s->next) {
        if (s->type != N_ASSIGN) continue;
        int f = 0;
        for (int i = 0; i < ns; i++) if (!strcmp(seen[i], s->name)) f = 1;
        if (!f) { seen[ns++] = s->name; fprintf(o, "    double %s = 0;\n", s->name); }
    }
    for (Node *s = prog->l; s; s = s->next) {
        if (s->type == N_ASSIGN) { fprintf(o, "    %s = ", s->name); gen_expr(s->l, o); fprintf(o, ";\n"); }
        else if (s->type == N_PRINT) { fprintf(o, "    printf(\"%%g\\n\", "); gen_expr(s->l, o); fprintf(o, ");\n"); }
    }
    fprintf(o, "    return 0;\n}\n");
}

/* ---------- AST -> JSON ---------- */
static void json_node(Node *n, FILE *o) {
    fprintf(o, "{\"name\":\"");
    switch (n->type) {
    case N_PROG:   fprintf(o, "Program"); break;
    case N_NUM:    fprintf(o, "%g", n->val); break;
    case N_VAR:    fprintf(o, "%s", n->name); break;
    case N_BIN:    fprintf(o, "%c", n->op); break;
    case N_NEG:    fprintf(o, "neg"); break;
    case N_ASSIGN: fprintf(o, "= %s", n->name); break;
    case N_PRINT:  fprintf(o, "print"); break;
    }
    fprintf(o, "\"");
    if (n->type == N_PROG) {
        fprintf(o, ",\"children\":[");
        for (Node *s = n->l; s; s = s->next) { json_node(s, o); if (s->next) fprintf(o, ","); }
        fprintf(o, "]");
    } else if (n->l) {
        fprintf(o, ",\"children\":[");
        json_node(n->l, o);
        if (n->r) { fprintf(o, ","); json_node(n->r, o); }
        fprintf(o, "]");
    }
    fprintf(o, "}");
}
void dump_json(Node *n, FILE *o) { json_node(n, o); fprintf(o, "\n"); }
