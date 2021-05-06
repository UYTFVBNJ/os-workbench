#include <common.h>
#include <pmm.h>

#define SLAB_MAX_NUM 5

#define SLAB_TOTAL_SHIFT (BUDDY_UNIT_SHIFT + 1)
#define SLAB_TOTAL_SIZE (1 << SLAB_TOTAL_SHIFT)
#define SLAB_UNIT_MAX_SHIFT (BUDDY_UNIT_SHIFT - 1)
#define SLAB_UNIT_MAX_SIZE (1 << SLAB_UNIT_MAX_SHIFT)

typedef struct {
  size_t UNIT_SHIFT, UNIT_SIZE, UNIT_NUM;
  int pos;
  int max_num, invalid_num;
  bool *valid;
  void *mem;
  int cpu;
} slab_block_t;

#define SLAB_BLOCK_INIT(unit_xft, cpu, slab_head)                           \
  ((slab_block_t){.UNIT_SHIFT = unit_xft,                                   \
                  .UNIT_SIZE = 1 << unit_xft,                               \
                  .UNIT_NUM = (SLAB_TOTAL_SIZE - sizeof(slab_block_t **)) / \
                              ((1 << unit_xft) + 1),                        \
                  .invalid_num = 0,                                         \
                  .pos = 0,                                                 \
                  .cpu = cpu})
