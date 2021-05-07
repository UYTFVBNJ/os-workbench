#include <buddy.h>
#include <common.h>
#include <minilib.h>
#include <pmm.h>

#define SLAB_MAX_NUM 5

#define SLAB_TOTAL_SHIFT (BUDDY_UNIT_SHIFT + 2)
#define SLAB_TOTAL_SIZE (1 << SLAB_TOTAL_SHIFT)
#define SLAB_UNIT_MAX_SHIFT (BUDDY_UNIT_SHIFT - 1)
#define SLAB_UNIT_MAX_SIZE (1 << SLAB_UNIT_MAX_SHIFT)

typedef struct {
  size_t UNIT_SHIFT, UNIT_SIZE, UNIT_NUM;
  int pos;
  int invalid_num;
  bool *valid;
  void *mem;
  int cpu;
} slab_block_t;
