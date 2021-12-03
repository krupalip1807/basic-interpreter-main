#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define K	1024

typedef enum { TRUE=1, FALSE=0 } bool;

// lex.c:
typedef enum {
  T_PLUS, T_MINUS, T_MULT, T_DIV, T_MOD, T_OR, T_AND, T_LT, T_GT, T_LTE, T_GTE, T_EQUAL, T_NEQUAL, 
  T_ASSIGN, T_OPAREN, T_CPAREN, 
  T_NUMBER, T_IDENTIFIER, T_IF, T_ELSE, T_WHILE, T_FOR, T_DO, T_SWITCH, T_CASE, T_DEFAULT, T_PRINT, T_BREAK, T_CONTINUE,
  T_OCBRACE, T_CCBRACE,
  T_EOS, T_EOI, T_UNKNOWN, T_COLON
} token_t;

struct cur_tok {
  token_t tok;
  char buf[K];
  int64_t value;
};

// expr.c:
typedef enum { NUMBER, OPERATOR } flag_t;
typedef enum { VALUE, VARIABLE, ENODE } value_t;
typedef struct var var_t;

struct op {
  token_t op;
  int unary, prec, dir;
};

struct var {
  char *name;
  int64_t value;
  var_t *next;
};

// lang.c:
typedef enum {
  ST_EXPRESSION, ST_WHILE, ST_FOR, ST_DO, ST_SWITCH, ST_CASE, ST_DEFAULT, ST_IF, ST_PRINT, ST_BREAK, ST_CONTINUE
} stmt_t;

typedef struct enode enode_t;
typedef struct statement stnode_t;

struct enode {
  token_t op;
  union {
    int64_t value;
    struct var *var;
  };
  enode_t *left, *right;
};

struct while_st {
  enode_t *expr;
  stnode_t *body, *bodyelse;
};

struct for_st {
  enode_t *expr1, *expr2, *expr3;
  stnode_t *body;
};

struct do_st {
  enode_t *expr;
  stnode_t *body;
};

struct switch_st {
  enode_t *expr;
  stnode_t *body;
};

struct case_st {
  enode_t *expr;
  stnode_t *body;
};

struct default_st {
  stnode_t *body;
};

struct if_st {
  enode_t *expr;
  stnode_t *body, *elsebody;
};

struct statement {
  stmt_t type;
  union {
    enode_t *expr;
    struct while_st *w;
    struct for_st *f;
    struct do_st *d;
    struct switch_st *s;
    struct case_st *c;
    struct default_st *df;
    struct if_st *i;
  };
  struct statement *next;
};

// Prototypes:
// lex.c:
void startlex(FILE *fp);
token_t lex(int64_t *value, char *buf);

// stack.c:
void push_enode(enode_t *e);
enode_t *pop_enode(void);
void push_op(token_t op, int unary, int prec, int dir);
struct op pop_op(void);
token_t peek_op(void);
int peek_prec(void);

// expr.c:
void die(char *why);
enode_t *expr(bool *empty);

// run.c:
int64_t eval_expr(enode_t *e);
void run(stnode_t *st, int *cont, int *brk);

// as.c
void assemble(stnode_t *st, char *contlbl, char *brklbl);

