#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define bool _Bool
#define true 1
#define false 0
#define BUFF 65536 

// this should be enough
static char buf[BUFF] = {}; // static type, by default, initialize to '\0'.
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

#define rest_buf(current) (BUFF - 1 - current)
static int current;

static inline uint32_t choose(uint32_t n){
  return rand() % n; 
}
static inline bool gen_num(){
  uint32_t num = choose(-1);  
  char num_str[12];  
  sprintf(num_str, "%u", num);
  int n = strlen(num_str);
  if (rest_buf(current) >= n){  
	for (int i = 0; i < n; i++)
      buf[current++] = num_str[i]; 
	return true;
  }
  return false;
}


static inline void opt_space(){
//int n = choose(10);
//if (rest_buf(current) < 10)
  if (rest_buf(current) > 0){
	int  n = choose(2);
    for (int i = 0; i < n; i++)
	  buf[current++] = ' ';
  }
}

static char op[] = {'+', '-', '*', '/'};
static inline bool gen_rand_expr() {
  //buf[0] = '\0';
  int temp = 0;
  switch (choose(4)){
	/*BNF recursive definition*/
    case 0: return gen_num(); 
	case 1: 
	  if (rest_buf(current) > 2){
		temp = current++;
	    if (gen_rand_expr() == true && rest_buf(current) > 0){ 
		  buf[temp] = '(';
		  buf[current++] = ')';
		  return true;
		}
		else current = temp; //roll back
	  }  
	  return false;
	case 2: 
	  temp = current;
	  if (gen_rand_expr() == true){
		buf[current++] = op[choose(4)];
	    if (gen_rand_expr() == true)
		  return true;
		current = temp;
	  }	
	  return false;
	
	default:
	  temp = current;
      opt_space();
	  if (gen_rand_expr() == true){
		opt_space();
		return true;
	  }
	  current = temp;
	  return false;
	
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
tag:
	current = 0;
    while (!gen_rand_expr())
	  current = 0;
	
	buf[current] = '\0';
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);
    int result;
    int match = fscanf(fp, "%d", &result);
    pclose(fp);
    if (match != 1){
	   printf("divide:%u %s\n", result, buf);
	   goto tag;	
	}
    printf("%u %s\n", result, buf);
  }
  return 0;
}
