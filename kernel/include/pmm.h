#include <common.h>
#include <spinlock.h>

typedef struct {
  spinlock_t lock;

  uint8_t *mem;
} segregated_block;


