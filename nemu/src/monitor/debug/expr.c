#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, 
  TK_EQ,

  /* TODO: Add more token types */
  TK_NUMBER,
  TK_HEX,
  TK_REG,
  TK_NEQ,
  TK_AND,
  TK_OR,
  TK_NEGATIVE,
  TK_DEREF
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"0x[0-9A-Fa-f][0-9A-Fa-f]*", TK_HEX},
  {"0|[1-9][0-9]*", TK_NUMBER},
  {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip|ax|cx|dx|bx|sp|bp|si|di|al|cl|dl|bl|ah|ch|dh|bh)", TK_REG},
  {"==", TK_EQ},         // equal
  {"!=", TK_NEQ},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"!", '!'},         
  {"-", '-'},
  {"\\*", '*'},
  {"\\/", '/'},
  {"\\(", '('},
  {"\\)", ')'},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

//match pair of parentheses
bool check_parentheses(int p, int q) {
  if(p >= q) {
    return false;
  }
  if(tokens[p].type != '(' || tokens[q].type != ')'){
    return false;
  }
  int count = 0;
  for(int i = p + 1; i < q; i++){
    if(tokens[i].type == '(') {
      count++;
    }
    if(tokens[i].type == ')') {
      if(count > 0){
        count--;
      }
      else {
        return false;
      }
    }
  }
  if(count == 0) {
    return true;
  }
  else {
    return false;
  }
}

// algorithm priority
int op_table[] = 
{5, // AND (&&)
  5, // OR (||)
  4, // EQ (==)
  4, // NEQ (!=)
  3, // ADD (+)
  3, // SUB (-)
  2, // MUL (*)
  2, // DIV (/)
  1, // NEGATIVE (-)
  1, // DEREF (*)
  1, // LOGICAL NOT (!)
};

int get_precedence(int op) {
  switch (op) {
    case TK_AND:      return op_table[0];
    case TK_OR:       return op_table[1];
    case TK_EQ:       return op_table[2];
    case TK_NEQ:      return op_table[3];
    case '+':         return op_table[4];
    case '-':         return op_table[5];
    case '*':         return op_table[6];
    case '/':         return op_table[7];
    case TK_NEGATIVE: return op_table[8];
    case TK_DEREF:    return op_table[9];
    case '!':         return op_table[10];
    default:          return -1; // 错误或未知操作符
  }
}

int findDominantOp(int p, int q) {
  int level = 0;
  int dominant_op = -1;
  int max_precedence = -1;

  for (int i = p; i < q; i++) {
      if (tokens[i].type == '(') {
          level++;
      }
      if (tokens[i].type == ')') {
          level--;
      }

      if (level == 0) {
          int precedence = get_precedence(tokens[i].type);
          
          if (precedence > max_precedence) {
              max_precedence = precedence;
              dominant_op = i;
          }
      }
  }

  if (dominant_op == -1) {
      printf("ERROR: No dominant operator found\n");
      assert(0);
  }

  return dominant_op;
}

int eval(int p, int q){
  // Bad expression
  if(p > q) {
    printf("Bad expression\n");
    assert(0);
  }

  // Single token
  else if(p == q) {
    int num;
    switch (tokens[p].type){
      case TK_NUMBER:
        sscanf(tokens[p].str, "%d", &num);
        return num;
      case TK_HEX:
        sscanf(tokens[p].str, "%x", &num);
        return num;
      case TK_REG:
        if(strcmp(tokens[p].str, "eip") == 0) {
          return cpu.eip;
        }
        for(int i = 0; i < 8; i++) {
          if(strcmp(tokens[p].str, regsl[i]) == 0) {
            return reg_l(i);
          }
          if(strcmp(tokens[p].str, regsw[i]) == 0) {
            return reg_w(i);
          }
          if(strcmp(tokens[p].str, regsb[i]) == 0) {
            return reg_b(i);
          }
        }
        printf("error in TK_REG\n");
        assert(0);
    }
  }

  else if(check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1);
  }

  else {
    int op = findDominantOp(p, q);
    vaddr_t addr;
    int result = 0;

    switch (tokens[op].type) {
      case TK_NEGATIVE:
        printf("Operator = -\n");
        result = eval(p + 1, q);
        printf("Value=%d\n",result);
        return -result;
      case TK_DEREF:
        addr = eval(p + 1, q);
        result = vaddr_read(addr, 4);
        printf("adddr=%u(0x%x)---->value=%d(0x%08x)\n", addr, addr, result, result);
        return result;
      case '!': 
        result = eval(p + 1, q);
        printf("Operator = !\n");
        if(result != 0) {
          printf("Value=0\n");
          return 0;
        }
        else {
          printf("Value=1\n");
          return 1;
        }
    }

    int val1 = eval(p, op - 1);
    int val2 = eval(op + 1, q);
    switch(tokens[op].type) {
      case '+':
        printf("Operator = +\n");
        printf("Value = %d\n",val1+val2);
        return val1 + val2;
      case '-': 
        printf("Operator = -\n");
        printf("Value = %d\n",val1-val2);
        return val1 - val2;
      case '/':
        if(val2==0){
          printf("Error: Division by zero\n");
          assert(0);
        }
        printf("Operator = /\n");
        printf("Value = %d\n",val1/val2);
        return val1 / val2;
      case '*':
        printf("Operator = *\n");
        printf("Value = %d\n",val1*val2);
        return val1 * val2;
      case TK_EQ:
        printf("Operator = ==\n");
        printf("Value = %d\n",val1==val2);
        return val1 == val2;
      case TK_NEQ: 
        printf("Operator = !=\n");
        printf("Value = %d\n",val1!=val2);
        return val1 != val2;
      case TK_AND: 
        printf("Operator = &&\n");
        printf("Value = %d\n",val1&&val2);
        return val1 && val2;
      case TK_OR: 
        printf("Operator = ||\n");
        printf("Value = %d\n",val1||val2);
        return val1 || val2;
      default:
        printf("Error: Invalid operator\n");
        assert(0);
    }
  }
  return 1;
}

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        if(substr_len > 32) {
          assert(0);
        }
        if(rules[i].token_type == TK_NOTYPE) {
          break;
        }
        else {
          tokens[nr_token].type = rules[i].token_type;
          switch (rules[i].token_type) {
          case TK_NUMBER:
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            *(tokens[nr_token].str + substr_len) = '\0';
            break;
          case TK_HEX:
            strncpy(tokens[nr_token].str, substr_start + 2, substr_len - 2);
            *(tokens[nr_token].str + substr_len - 2) = '\0';
            break;
          case TK_REG: 
            strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
            *(tokens[nr_token].str + substr_len - 1) = '\0';
          }
          nr_token += 1;
          break;
        }
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */

  for(int i = 0; i < nr_token; i++) {
    if(i == 0){
      if(tokens[0].type == '-') {
        tokens[0].type = TK_NEGATIVE;
      }
      if(tokens[0].type == '*') {
        tokens[0].type = TK_DEREF;
      }
    }
    if(tokens[i].type == '-') { 
      if(tokens[i - 1].type != TK_NUMBER && tokens[i - 1].type != ')') {
        tokens[i].type = TK_NEGATIVE;
      }
    }
    if(tokens[i].type == '*') {
      if(tokens[i - 1].type != TK_NUMBER && tokens[i - 1].type != ')') {
        tokens[i].type = TK_DEREF;
      }
    }
  }
  *success = true;
  return eval(0, nr_token - 1);
}
