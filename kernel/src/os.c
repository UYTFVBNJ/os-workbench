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

  iset(true);
  while (1);
}

static Context *os_trap(Event ev, Context *context) {
  assert(0);
}

static void *os_on_irq(int seq, Event ev, handler_t *handler) {

}

MODULE_DEF(os) = {
  .init = os_init,
  .run = os_run,
  .trap = os_trap,
};