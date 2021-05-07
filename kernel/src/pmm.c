#include <buddy.h>
#include <pmm.h>
#include <slab.h>

buddy_block_t buddy_block;

// framework
static void *kalloc(size_t size) {
  if (size <= SLAB_UNIT_MAX_SIZE) {
    void *ret = slab_alloc(size);
    if (ret != NULL) return ret;
  }
  return buddy_alloc(&buddy_block, size);
}

static void kfree(void *ptr) {
  if (((uintptr_t)ptr & (BUDDY_UNIT_SIZE - 1)) != 0 ||
      !buddy_check_alloced(&buddy_block, ptr))
    slab_free(ptr);
  else
    buddy_free(&buddy_block, ptr);
}

static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);

  // buddy_init(&buddy_block, heap.start, heap.end);

  buddy_init(&buddy_block, heap.start, heap.end);
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
#ifdef TEST
    .test = pmm_test,
#endif
};
