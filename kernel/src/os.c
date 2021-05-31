#include <common.h>
#include <os.h>

struct handler_se {
  handler_t handler;
  int event;
  int seq;
} handlers[MAX_HANDLERS];
int cnt_handlers = 0;

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

// Parallel
static Context *os_trap(Event ev, Context *context) {
  Context *ret = NULL;
  for (int i = 0; i < cnt_handlers; i ++) {
      struct handler_se *h = &handlers[i];
      if (h->event == EVENT_NULL || h->event == ev.event) {
      Context *c = h->handler(ev, context);
      panic_on(c && ret, "returning multiple contexts");
      if (c) ret = c;
    }
  }
  panic_on(!ret, "returning NULL context");
  // panic_on(sane_context(ret), "returning to invalid context");
  return ret;
}

static void os_on_irq(int seq, int event, handler_t handler) {
  TRACE_ENTRY;
  // sort
  int pos;
  for (pos = 0; pos < cnt_handlers && seq >= handlers[pos].seq; pos ++);

  for (int i = cnt_handlers ++; i > pos; i --) handlers[i] = handlers[i - 1];
  handlers[pos].seq     = seq;
  handlers[pos].event   = event;
  handlers[pos].handler = handler;

  for (int i = 0; i < cnt_handlers; i ++) printf("%d ", handlers[i].seq);
  printf("\n");
  TRACE_EXIT;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run = os_run,
  .trap = os_trap,
  .on_irq = os_on_irq
};