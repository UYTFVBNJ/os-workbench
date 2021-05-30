#include <common.h>

static void os_init() {
  pmm->init();
  // kmt->init(); // TODO
}

static void os_run() {
  for (const char* s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }

#ifdef L1_TEST
  pmm->test();
#endif

  while (1);
}

MODULE_DEF(os) = {
  .init = os_init,
  .run = os_run,
};