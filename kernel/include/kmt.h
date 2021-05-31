#include <common.h>

#define STACK_SIZE 4096

typedef struct task {
  int status;
  const char *name;
  struct task *next;
  Context   *context;
  uint8_t *stack;
} task;