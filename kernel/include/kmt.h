#include <common.h>

#define STACK_SIZE 4096

typedef union task {
  struct {
    int status;
    const char *name;
    union task *next;
    Context   *context;
  };
  uint8_t *stack;
} task;