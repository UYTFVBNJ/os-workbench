#include <slab.h>

extern buddy_block_t buddy_block;

slab_block_t* slabs[MAX_CPU][SLAB_UNIT_MAX_SHIFT][SLAB_MAX_NUM];

inline void
slab_init(slab_block_t* block, int unit_xft)
{
  void *start = block, *end = start + SLAB_TOTAL_SIZE;

  end--;

  block->UNIT_SHIFT = unit_xft;
  block->UNIT_SIZE = 1 << unit_xft;
  block->UNIT_NUM =
    (SLAB_TOTAL_SIZE - sizeof(slab_block_t)) / (block->UNIT_SIZE + sizeof(int));

  block->num = block->UNIT_NUM;
  block->cpu = cpu_current();

  block->mem = end - (block->UNIT_NUM << block->UNIT_SHIFT);
  block->stack = start + sizeof(slab_block_t);

  for (int i = 0; i < block->UNIT_NUM; i++)
    block->stack[i] = i * block->UNIT_SIZE;
}

inline slab_block_t*
slab_find_available(int sz_xft)
{
  int cpu = cpu_current();
  for (int i = 0; i < SLAB_MAX_NUM; i++) { // TODO
    slab_block_t** slab = &slabs[cpu][sz_xft][i];

    if (*slab == NULL) {
      *slab = buddy_alloc(&buddy_block, SLAB_TOTAL_SIZE);
#ifdef TEST_LOG
      printf("acuired new SLAB[%d][%d][%d] from BUDDY at %p\n",
             cpu,
             sz_xft,
             i,
             *slab);
#endif
      // assert(*slab != NULL);
      if (*slab == NULL)
        return NULL;
      slab_init(*slab, sz_xft);
      return *slab;
    } else if ((*slab)->num > 0) {
      /*
      if (i + 1 < SLAB_MAX_NUM &&
          (*slab)->invalid_num <= (*slab)->UNIT_NUM * max_load_factor / 2 &&
          slabs[cpu][sz_xft][i + 1] != NULL &&
          slabs[cpu][sz_xft][i + 1]->invalid_num == 0) {
#ifdef TEST_LOG
        printf("freeing old SLAB[%d][%d][%d] from BUDDY at %p\n",
               cpu,
               sz_xft,
               i + 1,
               slabs[cpu][sz_xft][i + 1]);
#endif
        // buddy_free(&buddy_block, slabs[cpu][sz_xft][i + 1]);
        // slabs[cpu][sz_xft][i + 1] = NULL;
        */
      // printf("SLAB[%d][%d] available at %d\n", cpu, sz_xft, i);
      return *slab;
    }
  }
  // printf("SLAB[%d][%d] no available\n", cpu, sz_xft);
  return NULL;
}

void*
slab_alloc(size_t size)
{
  int sz_xft = ceil_shift(size);

  slab_block_t* block = slab_find_available(sz_xft);

  if (block != NULL) {
    return block->mem + block->stack[--block->num];
  }
  return NULL;
}

void
slab_free(void* ptr)
{
  slab_block_t* block =
    (slab_block_t*)((uintptr_t)ptr & ~(SLAB_TOTAL_SIZE - 1));

#ifdef TEST
  // assert(block->UNIT_SIZE != 0);

  // if (block->cpu != cpu_current())
  // printf("warning: cpu %d is accessing cpu %d's slab\n", cpu_current(),
  //  block->cpu);

  // assert(((uintptr_t)ptr - (uintptr_t)block->mem) % block->UNIT_SIZE == 0);
#endif

  block->stack[block->num++] = (uintptr_t)ptr - (uintptr_t)block->mem;
}