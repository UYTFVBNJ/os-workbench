#include <common.h>
#include <spinlock.h>

typedef struct {
  spinlock_t lock;

  uint8_t *mem;
} buddy_block;

typedef struct {
  spinlock_t lock;

  uint8_t *mem;
} segregated_block;


#define BUDDY_BLOCK_INIT(name, cpu)     ((spinlock_t) { .locked = 0, .name = name, .cpu = cpu})
