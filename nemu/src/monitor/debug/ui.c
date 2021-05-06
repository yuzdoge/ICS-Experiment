#include <isa.h>
#include "expr.h"
#include "watchpoint.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>

void cpu_exec(uint64_t);
int is_batch_mode();
void wp_display();

void wp_display(){
  WP *current = get_next_wp(NULL);
  if (current == NULL){
	printf("no watchpoints\n");
	return;
  }
  printf("Num     " "what\n");
  for (; current; current = get_next_wp(current))
	printf("%8d-" "%s\n", current->NO, current->what);
}

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}


static int cmd_si(char *args){
  char *arg; 
  char *temp;
  unsigned long long n = 1;
  /*Only accept numeric argument, constant expression is not be allowed*/
  if (args && (arg = strtok(args, " "))){
	if (sscanf(arg, "%llu", &n) != 1){
	  printf("Argument %s is not numeric\n", arg);		
	  return 0;
	}
	if ((arg = strtok(NULL, "")) != NULL && sscanf(arg, "%ms", &temp) != -1){
	  printf("Too much argument `%s`\n", arg);	
	  free(temp);
	  return 0;
	}
  }
  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args);

static struct {
  char *name;
  void (*handler)();
} info_opt[] = {
  {"r", isa_reg_display},
  {"w", wp_display},
};

#define NR_INFO (sizeof(info_opt) / sizeof(info_opt[0])) 

static int cmd_info(char *args){
  char *arg = NULL;
  char *temp = NULL;
  if ((arg = strtok(NULL, " "))){
	if ((temp = strtok(NULL, " ")))
		goto fail;
	for (int i = 0; i < NR_INFO; i++){
	  if (strcmp(arg, info_opt[i].name) == 0){
		info_opt[i].handler();
	    return 0;
	  }
    }
	printf("Unknown option `%s`\n", arg);
	return 0;
  }	
fail:
  printf("Try `help info` for more information\n");
  return 0;
}


#define Len 4
#define END_ADDR(start_addr, n) \
  ((start_addr + Len * n) >= PMEM_BASE + PMEM_SIZE ) ? \
  (start_addr + (PMEM_BASE + PMEM_SIZE - start_addr) / Len) : (start_addr + Len * n)

static inline bool my_in_pmem(paddr_t addr) { return (addr >= PMEM_BASE) && (addr < PMEM_BASE + PMEM_SIZE); }

static int cmd_x(char *args){
  vaddr_t addr;
  vaddr_t end_addr;
  unsigned int n;
  bool success;
  char *temp = NULL;
  if (args == NULL || sscanf(args, "%u %ms", &n, &temp) != 2){
    printf("Try `help x` for more information\n");
    if (temp)
	  free(temp);
	return 0;
  }
  free(temp);
  temp = strtok(NULL," ");
  args = temp + strlen(temp) + 1;
  addr = (vaddr_t)(expr(args, &success));
  if (success){
	//whether it need to check addr by myself?
	if (my_in_pmem(addr)){
	  //printf("PMEM_SIZE = %#x, start_addr + Len * n = %#x\n", PMEM_SIZE, addr+Len*n); 
	  end_addr = END_ADDR(addr, n);
	  if (addr == end_addr)
		printf("Fail to return:not enough %d bytes\n", Len);
	  else
		for (; addr < end_addr; addr += Len)
		  printf(FMT_WORD":  "FMT_WORD"\n", addr, vaddr_read(addr, Len));
		//printf("n=%d, addr=%#x, end_addr=%#x\n", n, addr, end_addr);
	}
	else
	  printf("Cannot access address "FMT_WORD"\n", addr);	
  }	
  return 0;
}


static int cmd_p(char *args){
  char *temp = NULL;
  word_t eval;
  bool success;
  if (args == NULL || sscanf(args, "%ms", &temp) == -1){
	  printf("Try `help p` for more information\n");
	  if (temp)
		free(temp);
	  return 0;
  }
  free(temp);
  eval = expr(args, &success);
  if (success)
  printf("eval=%d, %x\n", eval, eval);
  return 0;
}

static int cmd_w(char *args){
  static int seq = 0;
  char *temp = NULL;
  word_t eval;
  bool success;
  if (args == NULL || sscanf(args, "%ms", &temp) == -1){
	  printf("Try `help p` for more information\n");
	  if (temp)
		free(temp);
	  return 0;
  }
  free(temp);
  eval = expr(args, &success);
  if (success){
    WP* wp = new_wp();
    wp->NO = seq++;	
	wp->val = eval;
	wp->what = (char*)malloc(strlen(args) + 1);
	assert(wp->what);
	strcpy(wp->what, args);
	printf("watchpoint %d: %s\n", wp->NO, wp->what);
  }
 return 0;
}


static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Step program until N instructions\n\tUsage: si [N]", cmd_si },
  { "info", "Show things about the program being debugged\n"
		    "\tUsage: info [r|w]\n"
			"\t\tr\tlist of all registers and their content\n"
			"\t\tw\tinformation of status of watchpoints", cmd_info },
  { "x", "Examine Memory\n"
		 "\tUsage: x N EXPR\n"
		 "\tEXPR is an expression indicating the start address to examine, N is the repeat count of 4 bytes", cmd_x },
  { "p", "exvaluate expression\n\tUsage: p EXPR\n\tEXPR is an expression", cmd_p}, 
  { "w", "Set watchpoint and the program will be paused when the expression involved change\n"
         "\tUsage: w EXPR\n\tEXPR is an expression", cmd_w },
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop() {
  if (is_batch_mode()) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
