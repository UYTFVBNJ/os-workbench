#include <slab.h>

#define max_load_factor 0.8
#define max_probe 2 * (1.0 / (1.0 - max_load_factor))
// #define max_probe (block->UNIT_NUM)

extern buddy_block_t buddy_block;

slab_block_t* slabs[MAX_CPU][SLAB_UNIT_MAX_SHIFT][SLAB_MAX_NUM];

void
slab_init(slab_block_t* block, int unit_xft)
{
  void *start = block, *end = start + SLAB_TOTAL_SIZE;

  block->UNIT_SHIFT = unit_xft;
  block->UNIT_SIZE = 1 << unit_xft;
  block->UNIT_NUM =
    (SLAB_TOTAL_SIZE - sizeof(slab_block_t)) / ((1 << unit_xft) + sizeof(bool));

  block->invalid_num = 0;
  block->pos = 0;
  block->cpu = cpu_current();

  block->mem = end - (block->UNIT_NUM << block->UNIT_SHIFT);
  block->valid = start + sizeof(slab_block_t);

  memset(block->valid, 0, sizeof(bool) * block->UNIT_NUM);
}

slab_block_t*
slab_find_available(int sz_xft)
{
  for (int i = 0; i < SLAB_MAX_NUM; i++) {
    slab_block_t** slab = &slabs[cpu_current()][sz_xft][i];

    if (*slab == NULL) {
#ifdef TEST_LOG
      printf(
        "acuiring new SLAB[%d][%d][%d] from BUDDY\n", cpu_current(), sz_xft, i);
#endif
      *slab = buddy_alloc(&buddy_block, SLAB_TOTAL_SIZE);
#ifdef TEST_LOG
      printf("acuired new SLAB[%d][%d][%d] from BUDDY at %p\n",
             cpu_current(),
             sz_xft,
             i,
             *slab);
#endif
      assert(*slab != NULL);
      slab_init(*slab, sz_xft);
      return *slab;
    } else if ((*slab)->invalid_num <= (*slab)->UNIT_NUM * max_load_factor) {
      if (i + 1 < SLAB_MAX_NUM &&
          (*slab)->invalid_num <= (*slab)->UNIT_NUM * max_load_factor / 2 &&
          slabs[cpu_current()][sz_xft][i + 1] != NULL &&
          slabs[cpu_current()][sz_xft][i + 1]->invalid_num == 0) {
#ifdef TEST_LOG
        printf("freeing old SLAB[%d][%d][%d] from BUDDY at %p\n",
               cpu_current(),
               sz_xft,
               i + 1,
               slabs[cpu_current()][sz_xft][i + 1]);
#endif
        // buddy_free(&buddy_block, slabs[cpu_current()][sz_xft][i + 1]);
        // slabs[cpu_current()][sz_xft][i + 1] = NULL;
      }
      return *slab;
    }
  }
  return NULL;
}

void*
slab_alloc(size_t size)
{
  int sz_xft = is_2_power(size) ? num2shift(size) : num2shift(size) + 1;

  slab_block_t* block = slab_find_available(sz_xft);

  if (block != NULL) {
    for (int i = 0; i < max_probe;
         i++, block->pos = (block->pos + 1) % block->UNIT_NUM)
      if (!block->valid[block->pos]) {
        block->valid[block->pos] = true;
        block->invalid_num++;

        void* ret = block->mem + ((block->pos) << block->UNIT_SHIFT);
        block->pos = (block->pos + 1) % block->UNIT_NUM;

        return ret;
      }
  }
  // printf("SLAB[%d][%d] failed\n", cpu_current(), sz_xft);
  return NULL;
}

void
slab_free(void* ptr)
{
  slab_block_t* block =
    (slab_block_t*)((uintptr_t)ptr & ~(SLAB_TOTAL_SIZE - 1));

  assert(block->UNIT_SIZE != 0);

#ifdef TEST
  // if (block->cpu != cpu_current())
  // printf("warning: cpu %d is accessing cpu %d's slab\n", cpu_current(),
  //  block->cpu);
#endif

  assert(((uintptr_t)ptr - (uintptr_t)block->mem) % block->UNIT_SIZE == 0);
  assert(block->valid[((uintptr_t)ptr - (uintptr_t)block->mem) >>
                      block->UNIT_SHIFT] == true);

  block->valid[((uintptr_t)ptr - (uintptr_t)block->mem) >> block->UNIT_SHIFT] =
    false;

  block->invalid_num--;
}