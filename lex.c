#include "lang.h"

FILE *input;

struct cur_tok _cur;
int _cstack[100];
int _csp = 0;

void startlex(FILE *fp)
{
  input = fp;
  _cur.tok = lex(&_cur.value, _cur.buf);
}

// Gets the next character, returns EOF (-1) on end of input, pops from the
// unget character stack first if it's not empty.
int get(void)
{
  if (_csp) return _cstack[--_csp];
  return fgetc(input);
}

// Unget a character, pushes it to the unget character stack:
void unget(int c)
{
  if (c != EOF) _cstack[_csp++] = c;
}

// If the next character matches c, then move past it and return TRUE,
// otherwise put it back and return FALSE
bool next(int c)
{
  int ch = get();
  if (ch == c) return TRUE;
  unget(ch);
  return FALSE;
}

token_t identifier(int c, char *buf)
{
  int p = 0;
  buf[p++] = c;
  for(c=get(); isalpha(c) || isdigit(c) || c == '_'; c = get())
    buf[p++] = c;
  buf[p] = '\0';
  unget(c);
  if (strcmp(buf, "if") == 0) return T_IF;
  if (strcmp(buf, "else") == 0) return T_ELSE;
  if (strcmp(buf, "while") == 0) return T_WHILE;
  if (strcmp(buf, "for") == 0) return T_FOR;
  if (strcmp(buf, "do") == 0) return T_DO;
  if (strcmp(buf, "switch") == 0) return T_SWITCH;
  if (strcmp(buf, "case") == 0) return T_CASE;
  if (strcmp(buf, "default") == 0) return T_DEFAULT;
  if (strcmp(buf, "print") == 0) return T_PRINT;
  if (strcmp(buf, "break") == 0) return T_BREAK;
  if (strcmp(buf, "continue") == 0) return T_CONTINUE;
  return T_IDENTIFIER;
}

/**
 * Returns the next token. Fills value in with the value of the number for
 * T_NUMBER tokens.
 */
token_t lex(int64_t *value, char *buf)
{
  int c, n;
  
  // Skip over any white-space:
  do {
    c = get();
  } while(isspace(c));

  switch(c) {
    case EOF: return T_EOI;
    case ';': return T_EOS;
    case '+': return T_PLUS;
    case '-': return T_MINUS;
    case '/': return T_DIV;
    case '%': return T_MOD;
    case '(': return T_OPAREN;
    case ')': return T_CPAREN;
    case '{': return T_OCBRACE;
    case '}': return T_CCBRACE;
    case '<': 
      if (next('=')) return T_LTE;
      return T_LT;
    case '>':  
      if (next('=')) return T_GTE;
      return T_GT;
    case '&':
      if (next('&')) return T_AND;
      return T_AND;
    case '|':
      if (next('|')) return T_OR;
      return T_OR;
    case '*':
      return T_MULT;
    case '=':
      if (next('=')) return T_EQUAL;
      return T_ASSIGN;
    case '!':
      if (next('=')) return T_NEQUAL;
      return T_UNKNOWN;
    default:
      if (isalpha(c) || c == '_') return identifier(c,buf);
      if (isdigit(c)) {
	n = 0;
	do {
	  n *= 10;
	  n += (c-'0');
	} while (isdigit(c = get()));
	unget(c);
	*value = n;
	return T_NUMBER;
      }
      return T_UNKNOWN;
  }

  // NOT REACHED
  return T_UNKNOWN;
}
