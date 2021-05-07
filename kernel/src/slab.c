#include <slab.h>

#define max_load_factor 0.8
#define max_probe 2 * (1.0 / (1.0 - max_load_factor))

extern buddy_block_t buddy_block;

slab_block_t *slabs[MAX_SMP][SLAB_UNIT_MAX_SHIFT][SLAB_MAX_NUM];

void slab_init(slab_block_t *block, int unit_xft) {
  void *start = block, *end = start + SLAB_TOTAL_SIZE;

  block->UNIT_SHIFT = unit_xft;
  block->UNIT_SIZE = 1 << unit_xft;
  block->UNIT_NUM = (SLAB_TOTAL_SIZE - sizeof(slab_block_t)) /
                    ((1 << unit_xft) + sizeof(bool));

  block->invalid_num = 0;
  block->pos = 0;
  block->cpu = cpu_current();

  block->mem = end - (block->UNIT_NUM << block->UNIT_SHIFT);
  *(slab_block_t **)start = block;
  block->valid = start + sizeof(slab_block_t);
}

slab_block_t *slab_find_available(int sz_xft) {
  for (int i = 0; i < SLAB_MAX_NUM; i++) {
    slab_block_t **slab = &slabs[cpu_current()][sz_xft][i];

    if (*slab == NULL) {
      *slab = buddy_alloc(&buddy_block, SLAB_TOTAL_SIZE);
      assert(*slab != NULL);
      slab_init(*slab, sz_xft);
      return *slab;
    } else if ((*slab)->invalid_num <= (*slab)->UNIT_NUM * max_load_factor) {
      return *slab;
    }
  }
  return NULL;
}

void *slab_alloc(size_t size) {
  int sz_xft = is_2_power(size) ? num2shift(size) : num2shift(size) + 1;

  slab_block_t *block = slab_find_available(sz_xft);

  if (block != NULL) {
    for (int i = 0; i < max_probe;
         i++, block->pos = (block->pos + 1) % block->UNIT_NUM)
      if (!block->valid[block->pos]) {
        block->valid[block->pos] = true;
        block->invalid_num++;
        return block->mem + (block->pos << block->UNIT_SHIFT);
      }
  }

  return NULL;
}

void slab_free(slab_block_t *block, void *ptr) {
  assert(((uintptr_t)ptr - (uintptr_t)block->mem) % block->UNIT_SIZE == 0);
  block->valid[((uintptr_t)ptr - (uintptr_t)block->mem) >> block->UNIT_SHIFT] =
      false;
  block->invalid_num--;

  if (block->invalid_num == 0) buddy_free(&buddy_block, block);
}