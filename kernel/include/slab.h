#include <buddy.h>
#include <common.h>
#include <minilib.h>
#include <pmm.h>

#define SLAB_MAX_NUM 10

#define SLAB_TOTAL_SHIFT (BUDDY_UNIT_SHIFT + 6)
#define SLAB_TOTAL_SIZE (1 << SLAB_TOTAL_SHIFT)
#define SLAB_UNIT_MAX_SHIFT (SLAB_TOTAL_SHIFT - 2)
#define SLAB_UNIT_MAX_SIZE (1 << SLAB_UNIT_MAX_SHIFT)

typedef struct
{
  size_t UNIT_SHIFT, UNIT_SIZE, UNIT_NUM;
  int num;
  int* stack;
  void* mem;
  int cpu;
} slab_block_t;

void*
slab_alloc(size_t size);
void
slab_free(void* ptr);