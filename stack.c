#include "lang.h"

// Value/variable/expression node stack:
enode_t *estack[K];
int esp = 0;

void push_enode(enode_t *e)
{
  if (esp == K) die("E-node stack overflow");
  estack[esp++] = e;
}

enode_t *pop_enode(void)
{
  if (esp <= 0) die("E-node stack underflow");
  return estack[--esp];
}


// Operator stack:
struct op opstack[K];
int osp = 0;

void push_op(token_t op, int unary, int prec, int dir)
{
  if (osp == K) die("Op stack overflow");
  opstack[osp++] = (struct op){.op=op, .unary=unary, .prec=prec, .dir=dir};
}

struct op pop_op(void)
{
  if (osp <= 0) die("Op stack underflow");
  return opstack[--osp];
}

token_t peek_op(void)
{
  if (osp <= 0) return T_UNKNOWN;
  return opstack[osp-1].op;
}

int peek_prec(void)
{
  if (osp <= 0) return -1;
  return opstack[osp-1].prec;
}
