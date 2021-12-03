#include "lang.h"

extern struct cur_tok _cur;
extern int osp, esp;

void die(char *why) {
  fprintf(stderr,"%s\n", why);
  exit(1);
}

//		 +,  -,  *,  /,  %, ||, &&,  <,  >, <=, >=, ==, !=,  =,  (,  )
int  prec[] = { 60, 60, 65, 65, 65, 20, 25, 50, 50, 50, 50, 45, 45, 10,  0,  0};
int uprec[] = { 75, 75, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int   dir[] = {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0};
int  udir[] = {  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

enode_t e_zero = {.op = T_NUMBER, .value = 0, .left = NULL, .right = NULL};

/**
 * "symbol table" for variables.
 */
var_t *vars = NULL;

struct var *lookup(char *name) {
  var_t *v;

  for(v=vars; v != NULL; v=v->next)
    if (strcmp(v->name, name) == 0) return v;
  v = malloc(sizeof(struct var));
  v->name = strdup(name);
  v->value = 0;
  v->next = vars;
  vars = v;
  
  return v;
}

enode_t *new_enode(token_t tok, int64_t value)
{
  enode_t *e = malloc(sizeof(enode_t));
  e->op = tok;
  e->value = value;
  e->left = e->right = NULL;
  return e;
}

void free_enode(enode_t *e)
{
  if (e->left) free_enode(e->left);
  if (e->right) free_enode(e->right);
  if (e != &e_zero) free(e);
}

/**
 * Performs an operation
 */
void action(void)
{
  struct op op;
  enode_t *l, *r;
  int64_t n;

  // Get the operator:
  op = pop_op();

  // Right side is at the top of the stack, left would be underneath it,
  // unless it's a unary op
  r = pop_enode();
  if (op.unary == FALSE) l = pop_enode();
  else l = &e_zero;

  if (uprec[op.op] == -1 && op.unary)
    die("Malformed unary expression in action.");

  // Perform the operation:

  // Perform constant folding if both sides of the operator are constants:
  if (l->op == T_NUMBER && r->op == T_NUMBER) {
    switch(op.op) {
      case T_PLUS:   	n = l->value  + r->value; break;
      case T_MINUS:  	n = l->value  - r->value; break;
      case T_MULT:   	n = l->value  * r->value; break;
      case T_DIV:    	n = l->value  / r->value; break;
      case T_MOD:    	n = l->value  % r->value; break;
      case T_OR:     	n = l->value || r->value; break;
      case T_AND:    	n = l->value && r->value; break;
      case T_LT:     	n = l->value  < r->value; break;
      case T_GT:     	n = l->value  > r->value; break;
      case T_LTE:    	n = l->value <= r->value; break;
      case T_GTE:    	n = l->value >= r->value; break;
      case T_EQUAL:  	n = l->value == r->value; break;
      case T_NEQUAL: 	n = l->value != r->value; break;
      default:
	break;
    }
    // Free the original enodes and push the folded result:
    free_enode(l);
    free_enode(r);
    push_enode(new_enode(T_NUMBER, n));
    return;
  }

  enode_t *e = new_enode(op.op, 0);
  e->left = l;
  if (op.op == T_ASSIGN && l->op != T_IDENTIFIER) die("Left side of assignment is not an lval.\n");
  e->right = r;
  push_enode(e);

  return;
}

enode_t *expr(bool *empty)
{
  enode_t *e;
  int stop = 0, parens = 0, p, d;

  flag_t flag = NUMBER;
  // Reset the stacks:
  osp = esp = 0;

  do {
    switch(_cur.tok) {
      case T_NUMBER:
	if (flag != NUMBER) die("Syntax error");
	push_enode(new_enode(T_NUMBER, _cur.value));
	flag = OPERATOR;
	break;
      case T_IDENTIFIER:
	e = new_enode(_cur.tok, 0);
	e->var = lookup(_cur.buf);
	push_enode(e);
	flag = OPERATOR;
	break;
      case T_OPAREN:
	parens++;
	push_op(T_OPAREN, 0, 0, 0);
	flag = NUMBER;
	break;
      case T_CPAREN:
	// A close parenthesis may signal the end of the expression:
	if (parens-- == 0) {
	  stop = 1;
	  break;
	}
	// Drain the op stack until it reaches it's matching open paren:
	while ((_cur.tok = peek_op()) != T_UNKNOWN) {
	  if (_cur.tok == T_OPAREN) break;
	  action();
	}
	if (_cur.tok == T_UNKNOWN) die("Mismatched parenthesis.");
	// Removes the opening paren:
	pop_op();
	flag = OPERATOR;
	break;
      case T_PLUS:
      case T_MINUS:
      case T_MULT:
      case T_DIV:
      case T_MOD:
      case T_OR:
      case T_AND:
      case T_LT:
      case T_GT:
      case T_LTE:
      case T_GTE:
      case T_EQUAL:
      case T_NEQUAL:
      case T_ASSIGN:
	// Get the precedence and direction of the current operator:
	p = (flag == NUMBER)? uprec[_cur.tok] : prec[_cur.tok];
	if (p == -1) die("Malformed expression, operator not a valid unary operator.");
	d = (flag == NUMBER)? udir[_cur.tok] : dir[_cur.tok];

	// Drain the op stack of operators of higher precedence (and equal to if
	// this op is left-assoc):
	if (d == 1) {	// Right associative
	  while (peek_prec() > p)
	    action();
	} else {	// Left associative
	  while (peek_prec() >= p)
	    action();
	}
	// Finally push the operator to the op stack:
	push_op(_cur.tok, flag==NUMBER, p, d);
	flag = NUMBER;
	break;
      default:
	stop = 1;
	break;
    }
  } while (!stop && ((_cur.tok = lex(&_cur.value,_cur.buf)) != T_EOI));

  // Drain the operator stack until it's empty:
  while(peek_op() != T_UNKNOWN)
    action();

  e = pop_enode();
  // The result of the entire expression should be the last and only value left:
  *empty = (esp == 0);
  return e;
}
