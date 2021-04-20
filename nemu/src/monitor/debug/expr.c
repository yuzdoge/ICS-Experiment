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


static bool legal_parentheses = true;  

static bool check_parentheses(int start, int end){
  bool flag = false; 

  /*legitimacy judgment of parentheses*/	
  int nr_bottom = 0; //times of touching the bottom of stack
  int stack_top = -1; // stack_top >=0 means that there are `(` in the stack
  for (int i = start; i <= end; i++){
    if (tokens[i].type == '('){
	  if (stack_top == -1)
        nr_bottom++;
	  ++stack_top;
	}
	else if (tokens[i].type == ')'){
	  if (stack_top < 0){
	    legal_parentheses = false;
		break;
	  }		
	  --stack_top;
	}
  }	
  if (stack_top >= 0)
    legal_parentheses = false;

  /*check whether the most left parentheses matches with the right parentheses*/
  if (legal_parentheses == true && tokens[start].type == '(' && tokens[end].type== ')' && nr_bottom == 1)
    flag = true;	  

  return flag;
}

static word_t eval(int start, int end){
  legal_parentheses = true;
  if (start > end){
  }
  else if (start == end){
    if (tokens[start].type != TK_DIGIT){
	  legal_parentheses = false;
	  goto err;
	}
	return 0;
  }
  else if (check_parentheses(start, end) == true){
    printf("match\n");  
	return 0;
  }
  else{
    if (legal_parentheses == false)
	  goto err;
	return 0;
  }
err:  
  printf("a syntax error in expression\n");
  return 0;
  
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  
  word_t result = eval(0, nr_token - 1); 
  *success = legal_parentheses;
  return result;
}
