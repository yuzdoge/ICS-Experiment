#include <isa.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

#define MAX_TOKENS 32 
enum {
  TK_NOTYPE = 256, TK_DIGIT, TK_DEREF, TK_HEX, TK_REG,
  TK_UNEQ = 271, TK_EQ, TK_AND, 
  /* TODO: Add more token types */
};

#define NR_BIN_OP 20
#define BOP_HASH(type) (type % NR_BIN_OP)
static char bop_priority[NR_BIN_OP] = {
  [BOP_HASH('+')] = 4, [BOP_HASH('-')] = 4,
  [BOP_HASH('*')] = 3, [BOP_HASH('/')] = 3,
  [BOP_HASH(TK_UNEQ)] = 7, [BOP_HASH(TK_EQ)] = 7,
  [BOP_HASH(TK_AND)] = 11,
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},				// spaces
  {"0[xX][[:xdigit:]]+", TK_HEX},		// hexadecimal digit, it must precede decimal digit to be checked. 
  {"[[:digit:]]+", TK_DIGIT},		// decimal digit
  {"\\+", '+'},						// plus
  {"-", '-'},						// minus
  {"\\*", '*'},						// multiply
  {"/", '/'},						// divide
  {"\\(", '('},						// left parenthesis
  {"\\)", ')'},						// right parenthesis
  {"\\$[a-zA-Z0-9]+", TK_REG},		// register
  {"==", TK_EQ},					// equal
  {"!=", TK_UNEQ},					// unequal
  {"&&", TK_AND}					// and
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

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

static Token tokens[MAX_TOKENS] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {

		if (nr_token >= MAX_TOKENS){
			printf("Too much tokens in the expression, which must be less than %d\n", MAX_TOKENS);
			return false;
		}

        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
		
        switch (rules[i].token_type) {
		  case '+': case '-': case '*': case '/': case '(': case ')': 
		  case TK_EQ: case TK_AND: case TK_UNEQ:  
			tokens[nr_token++].type = rules[i].token_type; break;
		  case TK_DIGIT: case TK_HEX: case TK_REG:
			if (substr_len >= 32){
				printf("Token at position %d is too long, which must be less than 32 characters\n"
					   "%s\n%*.s^\n", position, e, position, "");
				return false;
			}
			memcpy(tokens[nr_token].str, substr_start, substr_len);
			tokens[nr_token].str[substr_len] = '\0';
			tokens[nr_token++].type = rules[i].token_type;
			break;
		  case TK_NOTYPE: break;
          default: TODO();
        }
		
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  return true;
}


static bool islegal_parentheses(int start, int end){
  /*legitimacy judgment of parentheses*/	
 
  bool legal = true;
  int stack_top = -1;
  for (int i = start; i <= end; i++){
    if (tokens[i].type == '(')
	  ++stack_top;
	else if (tokens[i].type == ')'){
	  if (stack_top < 0)
	    return false;	
	  --stack_top;
	}
  }
  /* stack_top >=0 means that there are `(` in the stack*/
  if (stack_top >= 0) 
    legal = false;
  
  return legal;
}

static bool check_parentheses(int start, int end){
  bool flag = false; 
  //int nr_bottom = 0; //times of touching the bottom of stack
  int stack_top = 0; 

  if (tokens[start].type == '(' && tokens[end].type== ')'){
    for (int i = start + 1; i <= end; i++){
      if (tokens[i].type == '('){
	    if (stack_top == -1)
		  return false; // for instance (exp)+(exp)  
          //nr_bottom++;
	  ++stack_top;
	  }
	  else if (tokens[i].type == ')'){
	    --stack_top;
	  }
	}	
	flag = true;	  
  }

  return flag;
}

static int go_foward(int cursor){
  if (tokens[cursor].type == '('){
	int stack_top = 0;
	while (stack_top != -1){
		++cursor;	
		if (tokens[cursor].type == '(')
			++stack_top;
		else if (tokens[cursor].type == ')')
			stack_top--;
	}
  }
  return ++cursor; //point to the next atom
}

static inline bool is_op_type(int type){
  switch (type){
    case '+': case '-': case '*': case '/': 
	case TK_EQ: case TK_UNEQ: case TK_AND:
	  return true;
	default:
	  return false;
  }  
}

static int find_mainop(int start, int end){
#define NonOp false
#define Op true
  bool expected_op = NonOp;  
  int mop_pos = 0;  // the main operator has the lowest-priority (the highest value) in the expression
  for (int current = start; current <= end; current=go_foward(current)){
    switch (tokens[current].type){
	  case TK_AND: case TK_EQ: case TK_UNEQ:
	  case '+': case '-': case '*': case '/': 
		if (expected_op == NonOp)
		  return -1;
		if (bop_priority[BOP_HASH(tokens[current].type)] >= 
			bop_priority[BOP_HASH(tokens[mop_pos].type)])
		  mop_pos = current; 
		break;
	  default:
	    if (expected_op == Op)	
		  return -1;
		while ((current < end) && (tokens[current].type == TK_DEREF))
	      ++current;		
		if (is_op_type(tokens[current].type) || tokens[current].type == TK_DEREF)
		  return -1;
		else
		  go_foward(current);
	}
	  expected_op = ~expected_op; 
  }
  if (expected_op == NonOp)
	  return -1; // it means that the last atom is operater
  
  return mop_pos; // if mop equal zero, it means that the whole expression is deference 
   
}

static inline int atoui(char c){
  return c <= '9'? (c - '0'): ((c&0b11011111) - '7');  
}
static inline word_t strtoui(char *str, int base_n){
  word_t val = 0;
  for (int i = 0; str[i] != '\0'; i++)
    val = base_n*val + atoui(str[i]);
  return val;
}

static bool error_flag; 
#define report_err(...) \
  do { \
	printf(__VA_ARGS__); \
	error_flag = false; \
	return 0; \
  }while (0)

static word_t eval(int start, int end){
  if (error_flag == false)
	  return 0;

  word_t left_val, right_val;
  int mop_pos;
  vaddr_t addr;
  if (start > end){
	//for instance, `()` -> `` -> start > end
    report_err("a syntax error:start=%d > end=%d\n", start, end);
  } 
  else if (start == end){
    switch (tokens[start].type){
      case TK_DIGIT: return strtoui(tokens[start].str, 10); 
	  case TK_HEX: return strtoui(tokens[start].str + 2, 16);
	  case TK_REG: 
	    left_val = isa_reg_str2val(tokens[start].str + 1, &error_flag);
        if (error_flag == true) 
		  return left_val;
		report_err("register %s does not exist\n", tokens[start].str + 1);		
      default:	
	    report_err("a syntax error:the token at position %d is not digit\n", start);
     }
  }
  else if (check_parentheses(start, end) == true){
    printf("left parentheses %d matches with right parentheses %d\n", start, end);  
	return eval(start + 1, end - 1);
  }
  else{
	mop_pos = find_mainop(start, end);
    if (mop_pos == -1)
	  report_err("a systax error in the expression:missing mainop\n");	
    else if (mop_pos == 0){
	  addr = (vaddr_t)(eval(start + 1, end));
	  if ((addr >= PMEM_BASE) && (addr + sizeof(word_t) <= PMEM_BASE + PMEM_SIZE))
	    return vaddr_read(addr, sizeof(word_t));
	  else
	    report_err("cannot access address "FMT_WORD": illegal address or not enough %lu bytes\n", addr, sizeof(word_t));
       
	} 
	else{	
	  left_val = eval(start, mop_pos - 1);
	  right_val = eval(mop_pos + 1, end);
	  switch(tokens[mop_pos].type){
		  case '+': return left_val + right_val;
		  case '-': return left_val - right_val;
		  case '*': return left_val * right_val;
		  case '/': 
			if (right_val == 0)
			  report_err("divide by zero\n"); 
			return left_val / right_val;   
		  case TK_EQ: return left_val == right_val;
		  case TK_UNEQ: return left_val != right_val;
		  case TK_AND: return left_val && right_val;
		  default: assert(0);
	  }
    }
  }
}




word_t expr(char *e, bool *success){
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
 
  bool legal; 
  word_t result = 0;
  if ((legal = islegal_parentheses(0, nr_token - 1)) == true){
	error_flag = true;
	
	for (int i = 0; i < nr_token; i++){
	  if (tokens[i].type == '*' && (i == 0 || is_op_type(tokens[i - 1].type) || 
		   tokens[i - 1].type == '(' || tokens[i - 1].type == TK_DEREF))  
		tokens[i].type = TK_DEREF;
	}
	result = eval(0, nr_token - 1); 
  }
  else
	printf("a syntax error in the expression:ilegal_parentheses\n");

  *success = legal&error_flag;
  return result;
}
