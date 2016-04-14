/*
 *	Copyright (C) 2016 by Didier Derny
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "z80a.h"
#include "z80aglb.h"

#define T_NODE  0
#define T_VALUE 1

// character classes
#define C_BIN  0x0001
#define C_HEX  0x0002
#define C_OCT  0x0004
#define C_DEC  0x0008
#define C_OPE  0x0010
#define C_SPC  0x0020
#define C_SYM  0x0040

// parser fsm states
#define S_OPE  0
#define S_HEX  1
#define S_OCT  2
#define S_BIN  3
#define S_DEC  4
#define S_LIT  5
#define S_SYM  6

// standard operators: '(', ')', '*', '/', '%', '+', '-', '|', '&', '^', '~'
// other    operators: left shift '<', right shift '>', current position '$'
// other    tokens:    end of parsing 'X', numerical value 'N'

typedef struct node {
  int   op:8;
  int   left:8;
  int   right:8;
  int   flag:8;
  int   value;
} Node;

int getsymvalue(char *);
int getdot();

extern struct sym *get_sym(char *);
extern void asmerr(int);


static int  mknode(int, int, int, int);
static void parser_init(char *);
static int  parse();
static int  asctoi(char *);
static int  expr_factor();
static int  expr_term();
static int  expr_simple();
static int  expr_shift();
static int  expr_and();
static int  expr_xor();
static int  expr_or();
static int  expr();
static int  evaluate(int);
       int  eval(char *);

/* precedence
   operators        functions
   + - ~ (unaire)   expr_factor
   * /  %           expr_term
   + -              expr_simple
   < >              expr_shift
   &                expr_and
   ^                expr_xor
   |                expr_or 
   ( )              expr    
*/

#define MAXNODES 100

static int   parsed_pos  = 0;
static char *parsed_line = NULL;
static Node  nodes[MAXNODES];
static int   curnode = 0;
static int   charclass[128];
static int   code;
static int   type;
static int   value;

static int mknode(int op, int flag, int p1, int p2) {
  int  node;

  if (curnode < MAXNODES) {
    node = curnode;
    curnode++;
    nodes[node].op    = op;
    nodes[node].flag  = flag;      
    if (nodes[node].flag == T_NODE) {
      nodes[node].left  = p1;  
      nodes[node].right = p2;  
      nodes[node].value = 0;
    } else {
      nodes[node].left  = 0;  
      nodes[node].right = 0;  
      nodes[node].value = p1;
    }
    return node;
  }
  return -1;
}

// these 2 functions simulates the retieval 
// of a symbol or of the current position
int getdot() {
  return pc;
}

int getsymvalue(char *str) {
  struct sym *sp;
  
  if (strcasecmp(str, "$") == 0)
    return getdot();
  if (strlen(str) > SYMSIZE)
    str[SYMSIZE] = '\0';
  if ((sp = get_sym(str)) != NULL) 
    return sp->sym_val;
  else 
    asmerr(E_UNDSYM);
  return 0;
}

static void parser_init(char *str) {
  static int initialized = 0;
  int i;

  parsed_pos = 0;
  curnode = 1;
  bzero(&nodes[0], sizeof(nodes));
  if (parsed_line) 
    free(parsed_line);
  parsed_line = strdup(str);
  if (!initialized) {
    bzero(&charclass, sizeof(charclass));
    for (i='a'; i<='z'; i++)
      charclass[i] = charclass[i] | C_SYM;
    for (i='A'; i<='Z'; i++)
      charclass[i] = charclass[i] | C_SYM;
    for (i='0'; i<='9'; i++)
      charclass[i] = charclass[i] | C_DEC | C_HEX | C_SYM;
    for (i='0'; i<='7'; i++)
      charclass[i] |= C_OCT;
    for (i='a'; i<='f'; i++)
      charclass[i] |= C_HEX;
    for (i='A'; i<='F'; i++)
      charclass[i] |= C_HEX;
    for (i='0'; i<='1'; i++)
      charclass[i] |= C_BIN;
    charclass['_']  |= C_SYM;
    charclass['$']  |= C_SYM;
    charclass['-']  |= C_OPE;
    charclass['+']  |= C_OPE;
    charclass['*']  |= C_OPE;
    charclass['/']  |= C_OPE;
    charclass['%']  |= C_OPE;
    charclass['<']  |= C_OPE;
    charclass['>']  |= C_OPE;
    charclass['|']  |= C_OPE;
    charclass['&']  |= C_OPE;
    charclass['^']  |= C_OPE;
    charclass['~']  |= C_OPE;
    charclass['(']  |= C_OPE;
    charclass[')']  |= C_OPE;
    charclass[' ']  |= C_SPC;
    charclass['\n'] |= C_SPC;
    charclass['\r'] |= C_SPC;
    charclass['\t'] |= C_SPC;
    initialized = 1;
  }
}

static int parse() {
  int mark, state, index, quote, pcode, post; 
  char symbol[256];

  mark  = parsed_pos;
  state = -1;
 again:  
  parsed_pos = mark;
  state++;
  quote  = 0;  
  index  = 0;  
  code   = -1;
  pcode  = 0;
  value  = 0;
  post   = 0;
  while(1) {
    pcode = code;
  skip_spaces:
    code = parsed_line[parsed_pos]; 
    parsed_pos += (code) ? 1 : 0;
    type = ((code < 128) ? charclass[code] : 0);
    if ((quote == 0) && ((type & C_SPC) == C_SPC))
      goto skip_spaces;
    switch(state) {
    case S_OPE:
      if ((type & C_OPE) == C_OPE) 
        return code;
      goto again;

    case S_HEX:
      if ((type & C_HEX) == C_HEX) {
        value *= 16;
        value += toupper(code) - ((code <= '9') ? '0' : '7');
        index++;
        continue;
      } 
      if ((index > 0) && (toupper(code) == 'H')) {
        post = 1;
        continue;
      }
      if (post && ((code == 0) || ((type & C_OPE) == C_OPE))) {
        if ((type & C_OPE) == C_OPE) 
          parsed_pos--;
        return 'N';
      }
      goto again;
      
    case S_OCT:
      if ((type & C_OCT) == C_OCT) {
        value *= 8;
        value += code - '0';
        index++;
        continue;
      }
      if ((index > 0) && (toupper(code) == 'O')) {
        post = 1;
        continue;
      }
      if (post && ((code == 0) || ((type & C_OPE) == C_OPE))) {
        if ((type & C_OPE) == C_OPE) 
          parsed_pos--;
        return 'N';
      }
      goto again;

    case S_BIN:
      if ((type & C_BIN) == C_BIN) {
        value *= 2;
        value += code - '0';
        index++;
        continue;
      }
      if ((index > 0) && (toupper(code) == 'B')) {
        post = 1;
        return 'N';
      }
      if (post && ((code == 0) || ((type & C_OPE) == C_OPE))) {
        if ((type & C_OPE) == C_OPE) 
          parsed_pos--;
        return 'N';
      }
      goto again;
      
    case S_DEC:
      if ((type & C_DEC) == C_DEC) {
        value *= 10;
        value += code - '0';
        index++;
        continue;
      }
      if ((index > 0) && ((code == 0) || ((type & C_OPE) == C_OPE))) {
        if ((type & C_OPE) == C_OPE) 
          parsed_pos--;
        return 'N';
      }
      goto again;

    case S_LIT:
      if (quote == 0) {
        if (code != '\'')
          goto again;          
        quote = 1;
        continue;
      } else {
        if (code == '\'') { 
          if (pcode != '\\') {
            symbol[index] = '\0';
            value = asctoi(symbol);
            return 'N';
          } else 
            symbol[index++] = code;
        } else {
          symbol[index++] = code;
        }
      }
      break;

    case S_SYM:
      if ((type & C_SYM) == C_SYM) {
        symbol[index++] = code;
        continue;
      }
      if ((index > 0) && (((type & C_OPE) == C_OPE) || (code == 0))) {
        symbol[index] = '\0';        
        value = getsymvalue(symbol);
        if ((type & C_OPE) == C_OPE) 
          parsed_pos--;
        return 'N';
      }
      goto again;
      
    default:
      if (code == 0) 
        return 'X';
      asmerr(E_ILLOPE);
      return 1;
    }
  }
}

static int asctoi(char *str) {
  register int num;
  
  num = 0;
  while (*str) {
    num <<= 8;    
    if (*str == '\\') {
      str++;
      switch(*str++) {
      case 'a':  num |= 0x07; break;
      case 'b':  num |= 0x08; break;
      case 'f':  num |= 0x0c; break;
      case 'n':  num |= 0x0a; break;
      case 'r':  num |= 0x0d; break;
      case 't':  num |= 0x09; break;
      case 'v':  num |= 0x0b; break;
      case '\\': num |= 0x5c; break;
      case '\'': num |= 0x27; break;
      case '"':  num |= 0x22; break;
      case '?':  num |= 0x3f; break;
      default:
        break;
      }
    } else {
      num |= (int) *str++;
    }
  }
  return(num);
}

static int expr_factor() {
  int n1, tok;
  
  tok = parse();
  switch(tok) {
  case '(':  	
    n1 = expr();
    tok = parse();
    if (tok != ')') {
      asmerr(E_MISPAR);
      return 0;
    }
    return n1;
  case '~':
    return mknode('~', T_NODE, expr_factor(), 0);
  case '+':
    return mknode('*', T_NODE, mknode('N', T_VALUE,  1, 0), expr_factor());
  case '-':
    return mknode('*', T_NODE, mknode('N', T_VALUE, -1, 0), expr_factor());
  case 'N':
    return mknode('N', T_VALUE, value, 0);
  default :
    asmerr(E_ILLOPE);
    return 0;
  }
}

static int expr_term() {
  int n1, tok, mark;
  
  n1 = expr_factor();
  mark = parsed_pos;
  tok = parse();
  switch(tok) {
  case '/':
  case '*':
  case '%':
    return mknode(tok, T_NODE, n1, expr_term());
  default: 
    parsed_pos = mark;
    return n1;
  }
}

static int expr_simple() {
  int n1, tok, mark;

  n1 = expr_term();
  mark = parsed_pos;
  tok = parse();
  switch(tok) {
  case '+':
  case '-':
    return mknode(tok, T_NODE, n1, expr_simple());
  default: 
    parsed_pos = mark;
    return n1;
  }
}

static int expr_shift() {
  int n1, tok, mark;

  n1 = expr_simple();
  mark = parsed_pos;
  tok = parse();
  switch(tok) {
  case '<':
  case '>':
    return mknode(tok, T_NODE, n1, expr_shift());
  default :  
    parsed_pos = mark;
    return n1;
  }
}

static int expr_and() {
  int n1, tok, mark;

  n1 = expr_shift();
  mark = parsed_pos;
  tok = parse();
  switch(tok) {
  case '&':
    return mknode(tok, T_NODE, n1, expr_and());
  default :  
    parsed_pos = mark;
    return n1;
  }
}

int expr_xor() {
  int n1, tok, mark;

  n1 = expr_and();
  mark = parsed_pos;
  tok = parse();
  switch(tok) {
  case '^':
    return mknode(tok, T_NODE, n1, expr_xor());
  default :  
    parsed_pos = mark;
    return n1;
  }
}

static int expr_or() {
  int n1, tok, mark;

  n1 = expr_xor();
  mark = parsed_pos;
  tok = parse();
  switch(tok) {
  case '|':
    return mknode(tok, T_NODE, n1, expr_or());
  default :  
    parsed_pos = mark;
    return n1;
  }
}

static int expr() {
  return expr_or();
}

static int evaluate(int node) {
  int n1, n2;

  n1 = n2 = 0; // to make the compiler happy
  if (nodes[node].flag == 0) {
    if (nodes[node].left)
      n1 = evaluate(nodes[node].left);
    if (nodes[node].right)
      n2 = evaluate(nodes[node].right);
    switch(nodes[node].op) {
    case '~':
      return ~ n1;
    case '-':
      return n1 - n2;
    case '+':
      return n1 + n2;
    case '*':
      return n1 * n2;
    case '/':
      return n1 / n2;
    case '%':
      return n1 % n2;
    case '<':
      return n1 << n2;
    case '>':
      return n1 >> n2;
    case '|':
      return n1 | n2;
    case '^':
      return n1 ^ n2;
    case '&':
      return n1 & n2;
    default: 
      asmerr(E_ILLOPE);
      return n1;
    }
  } else {
    return nodes[node].value;
  }
}    

int eval(char *str) {
  parser_init(str);
  return evaluate(expr());
}

// from z80anum-orig.c

/*
 *	check value for range -256 < value < 256
 *	Output: value if in range, otherwise 0 and error message
 */
int chk_v1(int i)
{
  if (i >= -255 && i <= 255)
    return(i);
  else {
    asmerr(E_VALOUT);
    return(0);
  }
}

/*
 *	check value for range -128 < value < 128
 *	Output: value if in range, otherwise 0 and error message
 */
int chk_v2(int i)
{
  if (i >= -127 && i <= 127)
    return(i);
  else {
    asmerr(E_VALOUT);
    return(0);
  }
}
