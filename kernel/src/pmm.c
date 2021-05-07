#include <buddy.h>
#include <pmm.h>
#include <slab.h>

#define BUDDY_BLOCK_NUM 4
buddy_block_t buddy_block[BUDDY_BLOCK_NUM];

// framework
static void *kalloc(size_t size) {
  // if (size <= SLAB_UNIT_MAX_SIZE) {
  // void *ret = slab_alloc(size);
  // if (ret != NULL) return ret;
  // }
  return buddy_alloc(&buddy_block[cpu_current() % BUDDY_BLOCK_NUM], size);
}

static void kfree(void *ptr) {
  // if ((uintptr_t)ptr & (BUDDY_UNIT_SIZE - 1) != 0)
  // slab_free(ptr);
  // else
  buddy_free(&buddy_block[cpu_current() % BUDDY_BLOCK_NUM], ptr);
}

static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);

  // buddy_init(&buddy_block, heap.start, heap.end);

  size_t sz_pblock = pmsize / BUDDY_BLOCK_NUM;
  void *addr = heap.start;
  for (int i = 0; i < BUDDY_BLOCK_NUM; i++) {
    buddy_init(&buddy_block[i], addr, addr + sz_pblock);
    addr += sz_pblock;
  }
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
#ifdef TEST
    .test = pmm_test,
#endif
};
