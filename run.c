#include "lang.h"
int switchresult=0;
int switchmatch=0;

int64_t eval_expr(enode_t *e)
{
  int64_t l, r;

  // Don't "evaluate" the lval of an assignment.
  if (e->left && e->op != T_ASSIGN) l = eval_expr(e->left);
  if (e->right) r = eval_expr(e->right);

  switch(e->op) {
    case T_NUMBER:	return e->value;
    case T_IDENTIFIER:	return e->var->value;
    case T_PLUS:	return l + r;
    case T_MINUS:	return l - r;
    case T_MULT:	return l * r;
    case T_DIV:		return l / r;
    case T_MOD:		return l % r;
    case T_OR:		return l || r;
    case T_AND:		return l && r;
    case T_LT:		return l < r;
    case T_GT:		return l > r;
    case T_LTE:		return l <= r;
    case T_GTE:		return l >= r;
    case T_EQUAL:	return l == r;
    case T_NEQUAL:	return l != r;
    case T_ASSIGN:
      e->left->var->value = r;
      return r;
    default:
      return 0;
  }
  return 0;
}

void run(stnode_t *st, int *cont, int *brk)
{
  stnode_t *n;
  
  for(n = st; n != NULL; n=n->next) {
    switch(n->type) {
      case ST_EXPRESSION:
	eval_expr(n->expr);
	break;
      case ST_PRINT:
	printf("%ld\n", eval_expr(n->expr));
	break;
      case ST_CONTINUE:
	(*cont)++;
	return;
      case ST_BREAK:
	(*brk)++;
	return;
      case ST_WHILE:
	*brk = *cont = 0;
	int temp = 0;
	while(eval_expr(n->w->expr)) {
	    temp++;
	    run(n->w->body, cont, brk);
	    if (*brk) break;
	    if (*cont) { *cont = 0; continue; }
	}
	if(temp==0){
	    run(n->w->bodyelse, cont, brk);
	}	
	*brk = 0;	
	break;
      case ST_FOR:
	*brk = *cont = 0;
	for(eval_expr(n->f->expr1);eval_expr(n->f->expr2);eval_expr(n->f->expr3)){
	  run(n->f->body, cont, brk);
	  if (*brk) break;
	  if (*cont) { *cont = 0; continue; }	   
	}
	*brk = 0;
	break;
      case ST_DO:
	*brk = *cont = 0;
	do{
	  run(n->d->body, cont, brk);
	  if (*brk) break;
	  if (*cont) { *cont = 0; continue; }
	} while(eval_expr(n->d->expr));
	*brk = 0;
	break;
      case ST_SWITCH:
	*brk = *cont = 0;
	switchresult = eval_expr(n->s->expr);
	run(n->s->body, cont, brk);
	break;
      case ST_CASE:
	*brk = *cont = 0;
	switchmatch = 0;
	if(switchresult == eval_expr(n->c->expr)){
	   run(n->c->body, cont, brk);
	   switchmatch = 1;
	}
	break;
      case ST_DEFAULT:
	if(!switchmatch)
	   run(n->df->body, cont, brk);
	break;
      case ST_IF:
	if (eval_expr(n->i->expr)) {
	  run(n->i->body, cont, brk);
	} else if (n->i->elsebody) {
	  run(n->i->elsebody, cont, brk);
	}
	break;
    }
    if (*brk || *cont) return;
  }
}
