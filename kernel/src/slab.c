#include <slab.h>

#define load_factor 0.8
#define max_probe 1.0 / (1.0 - load_factor)

void slab_init(slab_block_t *block, void *start) {
  void *end = start + SLAB_TOTAL_SIZE;
  block->mem = end - (block->UNIT_NUM << block->UNIT_SHIFT);
  *(slab_block_t **)start = block;
  block->valid = (bool *)((slab_block_t **)start + 1);
  block->max_num = block->invalid_num * load_factor;
}

void *slab_alloc(size_t size) {
  /*
  for (int i = 0; i < SLAB_MAX_NUM; i++)
    // if ()
    slab_block_t
        *block for (int i = 0; i < max_probe;
                    i++, block->pos =
                             (block->pos + 1) %
                             block->UNIT_NUM) if (!block->valid[block->pos]) {
      block->valid[block->pos] = true;
      block->invalid_num++;
      return block->mem + (block->pos << block->UNIT_SHIFT);
    }
    */
  return NULL;
}

void slab_free(slab_block_t *block, void *ptr) {
  assert(((uintptr_t)ptr - (uintptr_t)block->mem) % block->UNIT_SIZE == 0);
  block->valid[((uintptr_t)ptr - (uintptr_t)block->mem) >> block->UNIT_SHIFT] =
      false;
  block->invalid_num--;
}