#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_DIGIT 

  /* TODO: Add more token types */
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
  {"-", '-'},			// minus
  {"\\*", '*'},         // multiply
  {"/", '/'},			// divide
  {"\\(", '('},			// left parenthesis
  {"\\)", ')'},			// right parenthesis
  {"==", TK_EQ},        // equal
  {"[[:digit:]]+", TK_DIGIT}		// decimal digit
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

static Token tokens[32] __attribute__((used)) = {};
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

		if (nr_token >= 32){
			printf("Too much tokens in the expression, which must be less than 32\n");
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
		  case '+': case '-': case '*': case '/': case '(': case ')': case TK_EQ:
			tokens[nr_token++].type = rules[i].token_type; break;
		  case TK_DIGIT: 
			if (substr_len >= 32){
				printf("Token at position %d is too long, which must be less than 32 characters\n"
					   "%s\n%*.s^\n", position, e, position, "");
				return false;
			}
			memcpy(tokens[nr_token].str, substr_start, substr_len);
			tokens[nr_token].str[substr_len + 1] = '\0';
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

static int find_mainop(int start, int end){
#define NonOp false
#define Op true
  bool expected_op = NonOp;  
  int mop_pos = 0; 
  for (int current = start; current <= end; current=go_foward(current)){
    switch (tokens[current].type){
	  case '+': case '-': 
		mop_pos = current; 
		/* it is still correct even though no `break` here, 
		 * because once this statement excuted,
		 * the first `if statement` of the next cases will not be true.
		 */
	  case '*': case '/': 
		if (tokens[mop_pos].type != '+' && tokens[mop_pos].type != '-') 
		  mop_pos = current;
		if (expected_op == NonOp)
		  return -1;
	    break;
	  default:
	    if (expected_op == Op)	
		  return -1;
	}
	  expected_op = ~expected_op; 
  }
  if (expected_op == NonOp)
	  return -1; // it means that the last atom is operater
  
  return mop_pos; 
  /* mop will never be zero, because it is an error that, 
   * there is no operator,and it will return -1 above.
   */
}

static word_t strtoui(char *str){
  word_t val = 0;
  for (int i = 0; str[i] != '\0'; i++)
    val = 10*val + (str[i] - '0'); 
  return val;
}

static bool error_flag; 

static word_t eval(int start, int end){
  if (error_flag == false)
	  return 0;

  word_t left_val, right_val;
  int mop_pos;
  if (start > end){
	//for instance, `()` -> `` -> start > end
    printf("a syntax error:start=%d > end=%d\n", start, end);
	error_flag = false;
	return 0;
  }
  else if (start == end){
    if (tokens[start].type != TK_DIGIT){
	  printf("a syntax error:the token at position %d is not digit\n", start);
	  error_flag = false;
	  return 0;
	}
	return strtoui(tokens[start].str);
  }
  else if (check_parentheses(start, end) == true){
    printf("left parentheses %d matches with right parentheses %d\n", start, end);  
	return eval(start + 1, end - 1);
  }
  else{
    if ((mop_pos = find_mainop(start, end)) == -1){
	  printf("a systax error in the expression:missing mainop\n");	
	  error_flag = false;
	  return 0;	
	}
	left_val = eval(start, mop_pos - 1);
	right_val = eval(mop_pos + 1, end);

	switch(tokens[mop_pos].type){
	  case '+': return left_val + right_val;
	  case '-': return left_val - right_val;
	  case '*': return left_val * right_val;
	  case '/': 
		if (right_val == 0)
		{	
		  printf("divide by zero\n"); 
		  error_flag = false;
		  return 0; 
		}
		return left_val / right_val;   
	  default: assert(0);
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
	result = eval(0, nr_token - 1); 
  }
  else
	printf("a syntax error in the expression:ilegal_parentheses\n");

  *success = legal&error_flag;
  return result;
}
