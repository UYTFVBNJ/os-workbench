#include <common.h>

typedef struct cpu {
  int          status;
  int          num;
  Context     *context;
  int noff;
} cpu_t;

#define STACK_SIZE 4096

struct task {
  int          status;
  const char  *name;
  struct task *next;
  void        (*entry)(void *);
  Context     *context;
  uint8_t     *stack;
};