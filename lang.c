#include "lang.h"

extern struct cur_tok _cur;
var_t *vars;

char *tokstr[] = {
  "+", "+=", "-", "*", "/", "%", "||", "&&", "<", ">", "<=", ">=", "==", "!=", "=", "(", ")",
  "#", "var", "if", "else", "while", "for", "do", "print", "break", "continue",
  "{", "}", ";", "EOI", "?", ":"
};

stnode_t *new_stnode(stmt_t type)
{
  stnode_t *n = malloc(sizeof(stnode_t));
  n->type = type;
  switch(type) {
    case ST_WHILE:
      n->w = malloc(sizeof(struct while_st));
      break;
    case ST_FOR:
      n->f = malloc(sizeof(struct for_st));
      break;
    case ST_DO:
      n->d = malloc(sizeof(struct do_st));
      break;
    case ST_CASE:
      n->c = malloc(sizeof(struct case_st));
      break;
    case ST_IF:
      n->i = malloc(sizeof(struct if_st));
      break;
    default:
      break;
  }
  n->next = NULL;
  return n;
}

void match(token_t tok)
{
  if (tok == _cur.tok) _cur.tok = lex(&_cur.value, _cur.buf);
  else {
    printf("Syntax error. Expected %s, got %s\n", tokstr[tok], tokstr[_cur.tok]);
    exit(1);
  }
}

int accept(token_t tok)
{
  if (_cur.tok == tok) {
    match(tok);
    return TRUE;
  }
  return FALSE;
}

stnode_t *parse_stmt(void) {
  bool empty;
  stnode_t *hp = NULL, *ep = NULL, *st;

  switch(_cur.tok) {
    case T_WHILE:
      match(T_WHILE);
      st = new_stnode(ST_WHILE);
      match(T_OPAREN);
      st->w->expr = expr(&empty);
      match(T_CPAREN);
      st->w->body = parse_stmt();
      match(T_ELSE);
      st->w->bodyelse = parse_stmt();
      break;
    case T_FOR:
      match(T_FOR);
      st = new_stnode(ST_FOR);
      match(T_OPAREN);
      st->f->expr1 = expr(&empty);
      match(T_EOS);
      st->f->expr2 = expr(&empty);
      match(T_EOS);
      st->f->expr3 = expr(&empty);
      match(T_CPAREN);
      st->f->body = parse_stmt();
      break;
    case T_DO:
      match(T_DO);
      st = new_stnode(ST_DO);
      st->d->body = parse_stmt();
      match(T_WHILE);
      match(T_OPAREN);
      st->d->expr = expr(&empty);
      match(T_CPAREN);
      match(T_EOS);
      break;
    case T_SWITCH:
      match(T_SWITCH);
      st = new_stnode(ST_SWITCH);
      match(T_OPAREN);
      st->s->expr = expr(&empty);
      match(T_CPAREN);
      st->s->body = parse_stmt();
      break;
    case T_CASE:
      match(T_CASE);
      st = new_stnode(ST_CASE);
      st->c->expr = expr(&empty);
      match(T_COLON);
      st->c->body = parse_stmt();
      break;
    case T_DEFAULT:
      match(T_DEFAULT);
      st = new_stnode(ST_DEFAULT);
      match(T_COLON);
      st->df->body = parse_stmt();
      break;
    case T_IF:
      match(T_IF);
      st = new_stnode(ST_IF);
      match(T_OPAREN);
      st->i->expr = expr(&empty);
      match(T_CPAREN);
      st->i->body = parse_stmt();
      st->i->elsebody = NULL;
      if (accept(T_ELSE)) {
	st->i->elsebody = parse_stmt();
      }
      break;
    case T_PRINT:
      match(T_PRINT);
      st = new_stnode(ST_PRINT);
      match(T_OPAREN);
      st->expr = expr(&empty);
      match(T_CPAREN);
      match(T_EOS);
      break;
    case T_BREAK:
      match(T_BREAK);
      st = new_stnode(ST_BREAK);
      match(T_EOS);
      break;
    case T_CONTINUE:
      match(T_CONTINUE);
      st = new_stnode(ST_CONTINUE);
      match(T_EOS);
      break;
    case T_OCBRACE:
      match(T_OCBRACE);
      while (!accept(T_CCBRACE)) {
	st = parse_stmt();
	if (hp == NULL) hp = ep = st;
	else {
	  ep->next = st;
	  ep = st;
	}
      }
      st = hp;
      break;
    default:
      st = new_stnode(ST_EXPRESSION);
      st->expr = expr(&empty);
      match(T_EOS);
      break;
  }

  return st;
}


void catfile(char *path)
{
  char buf[K];
  FILE *fp = fopen(path, "r");
  if (fp == NULL) return;

  while (fgets(buf, K, fp) != NULL)
    printf("%s", buf);

  fclose(fp);
  return;
}

int main(int argc, char *argv[])
{
  FILE *fp = stdin;
  stnode_t *hp = NULL, *ep = NULL, *st;

  if (argc > 1) {
    if ((fp = fopen(argv[1], "r")) == NULL) {
      perror("fopen");
      exit(1);
    }
  }

  startlex(fp);

  while(_cur.tok != T_EOI) {
    st = parse_stmt();
    if (hp == NULL) hp = ep = st;
    else {
      ep->next = st;
      ep = st;
    }
  }

  if (fp != stdin) fclose(fp);

#ifdef INTERPRETER
  int cont = 0, brk = 0;
  run(hp, &cont, &brk);
#else
  printf("%%include \"syscalls.h\"\n");
  printf("extern printnumnl, exit\n");

  printf("SECTION .data\n");
  for(var_t *v = vars; v; v=v->next) {
    printf("\t%s: dq 0\n", v->name);
  }

  printf("\nSECTION .text\n");

  printf("\nGLOBAL _start\n_start:\n");
  assemble(hp, NULL, NULL);
  printf("\ncall\texit\n");
#endif

  return 0;
}
