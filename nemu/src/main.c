void init_monitor(int, char *[]);
void engine_start();
int is_exit_status_bad();

#define TEST_EXPR
#include "monitor/debug/expr.h"  

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  init_monitor(argc, argv);
#ifdef TEST_EXPR
#define BUFSIZE (100 + 20) 
	char buf[BUFSIZE];
	FILE *fp = fopen("~/ics2020/nemu/tools/gen-expr/input", "r");
	assert(fp);
	while (fgets(buf, BUFSIZE, fp)){
	  buf[strlen(buf) - 1] = '\0'; 
	  printf("%s\n", buf);
	}
	fclose(fp);
#else
  /* Start engine. */
  engine_start();
#endif
  return is_exit_status_bad();
}
