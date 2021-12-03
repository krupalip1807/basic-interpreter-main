#include "lang.h"

int labelnum = 0;

char *newlabel(char *prefix)
{
  char buf[K];
  sprintf(buf, "%s%d", prefix, labelnum++);
  return strdup(buf);
}

void asm_expr(enode_t *e)
{
  char *l1, *l2;

  // Leaf nodes are "push" only:
  switch(e->op) {
    case T_NUMBER:
      printf("\tPUSH	%ld\n", e->value);
      return;
    case T_IDENTIFIER:
      printf("\tMOV	rax, [%s]\n", e->var->name);
      printf("\tPUSH	rax\n\n");
      return;
    default:
      break;
  }
      
  // Don't "evaluate" the lval of an assignment.
  if (e->left && e->op != T_ASSIGN) asm_expr(e->left);
  if (e->right) asm_expr(e->right);

  if (e->op != T_ASSIGN) printf("\tPOP	rbx\n");
  printf("\tPOP	rax\n");
  switch(e->op) {
    case T_PLUS:
      printf("\tADD	rax, rbx\n");
      break;
    case T_MINUS:
      printf("\tSUB	rax, rbx\n");
      break;
    case T_MULT:
      printf("\tIMUL	rbx\n");
      break;
    case T_DIV:
      printf("\tCQO\n");
      printf("\tIDIV	rbx\n");
      break;
    case T_MOD:
      printf("\tCQO\n");
      printf("\tIDIV	rbx\n");
      printf("\tMOV	rax, rdx\n");
      break;
    case T_OR:
      l1 = newlabel(".nz");
      l2 = newlabel(".f");
      printf("\tCMP	rax, 0\n");
      printf("\tJNE	%s\n",l1);
      printf("\tCMP	rbx, 0\n");
      printf("\tJNE	%s\n",l1);
      printf("\tMOV	rax, 0\n");
      printf("\tJMP	%s\n",l2);
      printf("%s:\n",l1);
      printf("\tMOV	rax, 1\n");
      printf("%s:\n",l2);
      free(l1);
      free(l2);
      break;
    case T_AND:
      l1 = newlabel(".z");
      l2 = newlabel(".f");
      printf("\tCMP	rax, 0\n");
      printf("\tJE	%s\n",l1);
      printf("\tCMP	rbx, 0\n");
      printf("\tJE	%s\n",l1);
      printf("\tMOV	rax, 1\n");
      printf("\tJMP	%s\n",l2);
      printf("%s:\n",l1);
      printf("\tMOV	rax, 0\n");
      printf("%s:\n",l2);
      free(l1);
      free(l2);
      break;
    case T_LT:
      l1 = newlabel(".lt");
      l2 = newlabel(".lt");
      printf("\tCMP	rax, rbx\n");
      printf("\tJL	%s\n", l1);
      printf("\tMOV	rax, 0\n");
      printf("\tJMP	%s\n", l2);
      printf("%s:\n", l1);
      printf("\tMOV	rax, 1\n");
      printf("%s:\n", l2);
      free(l1); free(l2);
      break;
    case T_GT:
      l1 = newlabel(".gt");
      l2 = newlabel(".gt");
      printf("\tCMP	rax, rbx\n");
      printf("\tJG	%s\n", l1);
      printf("\tMOV	rax, 0\n");
      printf("\tJMP	%s\n", l2);
      printf("%s:\n", l1);
      printf("\tMOV	rax, 1\n");
      printf("%s:\n", l2);
      free(l1); free(l2);
      break;
    case T_LTE:
      l1 = newlabel(".lte");
      l2 = newlabel(".lte");
      printf("\tCMP	rax, rbx\n");
      printf("\tJLE	%s\n", l1);
      printf("\tMOV	rax, 0\n");
      printf("\tJMP	%s\n", l2);
      printf("%s:\n", l1);
      printf("\tMOV	rax, 1\n");
      printf("%s:\n", l2);
      free(l1); free(l2);
      break;
    case T_GTE:
      l1 = newlabel(".gte");
      l2 = newlabel(".gte");
      printf("\tCMP	rax, rbx\n");
      printf("\tJGE	%s\n", l1);
      printf("\tMOV	rax, 0\n");
      printf("\tJMP	%s\n", l2);
      printf("%s:\n", l1);
      printf("\tMOV	rax, 1\n");
      printf("%s:\n", l2);
      free(l1); free(l2);
      break;
    case T_EQUAL:
      l1 = newlabel(".eq");
      l2 = newlabel(".eq");
      printf("\tCMP	rax, rbx\n");
      printf("\tJE	%s\n", l1);
      printf("\tMOV	rax, 0\n");
      printf("\tJMP	%s\n", l2);
      printf("%s:\n", l1);
      printf("\tMOV	rax, 1\n");
      printf("%s:\n", l2);
      free(l1); free(l2);
      break;
    case T_NEQUAL:
      l1 = newlabel(".neq");
      l2 = newlabel(".neq");
      printf("\tCMP	rax, rbx\n");
      printf("\tJNE	%s\n", l1);
      printf("\tMOV	rax, 0\n");
      printf("\tJMP	%s\n", l2);
      printf("%s:\n", l1);
      printf("\tMOV	rax, 1\n");
      printf("%s:\n", l2);
      free(l1); free(l2);
      break;
    case T_ASSIGN:
      printf("\tMOV	[%s], rax\n", e->left->var->name);
      break;
    default:
      break;
  }
  printf("\tPUSH	rax\n\n");
}


void assemble(stnode_t *st, char *contlbl, char *brklbl)
{
  char *l1, *l2;
  char *l3;
  stnode_t *n;

  for(n = st; n; n=n->next) {
    switch(n->type) {
      case ST_EXPRESSION:
	asm_expr(n->expr);
	printf("\tPOP	rax\n");
	break;
      case ST_PRINT:
	asm_expr(n->expr);
	printf("\tPOP	rax\n");
	printf("\tCALL	printnumnl\n\n");
	break;
      case ST_CONTINUE:
	if (contlbl)
	  printf("\tJMP	%s\n", contlbl);
	else die("No loop to break from.\n");
	break;
      case ST_BREAK:
	if (brklbl)
	  printf("\tJMP	%s\n", brklbl);
	else die("No loop to break from.\n");
	break;
      case ST_WHILE:
	l1 = newlabel(".while");
	l2 = newlabel(".wend");
	l3 = newlabel(".welse");
	asm_expr(n->w->expr);
	printf("\tPOP	rax\n");
	printf("\tCMP	rax, 0\n");
	printf("\tJLE	%s\n\n", l3);
	assemble(n->w->body, l1, l2);
	printf("%s:\n", l1);
	asm_expr(n->w->expr);
	printf("\tPOP	rax\n");
	printf("\tCMP	rax, 0\n");
	printf("\tJE	%s\n\n", l2);
	assemble(n->w->body, l1, l2);
	printf("\tJMP	%s\n", l1);
	printf("%s:\n", l3);
	assemble(n->w->bodyelse, l1, l2);
	printf("%s:\n", l2);
	break;
      case ST_FOR:
	l1 = newlabel(".for");
	l2 = newlabel(".fend");
	asm_expr(n->f->expr1);
	printf("%s:\n", l1);
	asm_expr(n->f->expr2);
	printf("\tPOP	rax\n");
	printf("\tCMP	rax, 0\n");
	printf("\tJE	%s\n\n", l2);
	assemble(n->w->body, l1, l2);
	asm_expr(n->f->expr3);
	printf("\tPOP	rax\n");
	printf("\tDEC	rax\n");
	printf("\tJMP	%s\n", l1);
	break;
      case ST_DO:
	l1 = newlabel(".do");
	l2 = newlabel(".dend");
	printf("%s:\n", l1);
	assemble(n->d->body, l1, l2);
	asm_expr(n->d->expr);
	printf("\tPOP	rax\n");
	printf("\tCMP	rax, 0\n");
	printf("\tJE	%s\n\n", l2);
	printf("\tJMP	%s\n\n", l1);
	printf("%s:\n", l2);
	break;
      case ST_SWITCH:
	asm_expr(n->s->expr);
	assemble(n->s->body, l1, l2);
	printf("\tPOP	rax\n");
	printf("\tMOV	rbx, rax\n");
	break;
      case ST_CASE:
	l1 = newlabel(".case");
	l2 = newlabel(".casend");
	asm_expr(n->c->expr);
	printf("\tPOP	rax\n");
	printf("\tCMP	rax, rbx\n");
	printf("\tJNE	rax, %s\n", l1);
	assemble(n->c->body, l1, l2);
	printf("%s:\n", l1);
	break;
      case ST_DEFAULT:
	assemble(n->d->body, l1, l2);
	break;
      case ST_IF:
	l1 = newlabel(".ifend");
	if (n->i->elsebody) l2 = newlabel(".else");

	asm_expr(n->i->expr);
	printf("\tPOP	rax\n");
	printf("\tCMP	rax, 0\n");
	printf("\tJE	%s\n", n->i->elsebody ? l2 : l1);
	assemble(n->i->body,contlbl,brklbl);
	if (n->i->elsebody) {
	  printf("%s:\n", l2);
	  assemble(n->i->elsebody,contlbl,brklbl);
	}
	printf("%s:\n", l1);
	free(l1);
	if (n->i->elsebody) free(l2);
	break;
    }
  }
}
