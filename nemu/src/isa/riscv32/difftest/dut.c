#include <isa.h>
#include <monitor/difftest.h>
#include "../local-include/reg.h"
#include "difftest.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  return memcmp(ref_r, &cpu, sizeof(CPU_state)) == 0 ? false : false;
}

void isa_difftest_attach() {
}
