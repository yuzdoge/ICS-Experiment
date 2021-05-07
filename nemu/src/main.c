void init_monitor(int, char *[]);
void engine_start();
int is_exit_status_bad();

#define TEST_EXPR
#include "monitor/debug/expr.h"  

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  init_monitor(argc, argv);
#ifdef TEST_EXPR
#define BUFSIZE (65536 + 20) 
	char buf[BUFSIZE];
	word_t eval, val;
	bool success;
	char *arg;
	int i = 1;
	FILE *fp = fopen("tools/gen-expr/input", "r");
	assert(fp);
	while (fgets(buf, BUFSIZE, fp)){
	  buf[strlen(buf) - 1] = '\0'; 
	  arg = strtok(buf," ");
	  sscanf(arg,"%u", &val);
	  arg = strtok(NULL, "");
	  eval = expr(arg, &success);
	  //assert(success);
	  if (val != eval){
	    printf("EXPR %d error: val=%u  eval=%u\n""%s\n", i, val, eval, arg);
		//assert(0);
	  }
	  else
		printf("EXPR %d pass\n", i);
	  i++;
	}
	fclose(fp);
#else
  /* Start engine. */
  engine_start();
#endif
  return is_exit_status_bad();
}
