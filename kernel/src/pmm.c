#include <buddy.h>
#include <pmm.h>
#include <slab.h>
buddy_block_t buddy_block[MAX_CPU];

// framework
static void *kalloc(size_t size) {
  // if (size <= SLAB_UNIT_MAX_SIZE) {
  // void *ret = slab_alloc(size);
  // if (ret != NULL) return ret;
  // }
  return buddy_alloc(&buddy_block[cpu_current() % 2], size);
}

static void kfree(void *ptr) {
  // if ((uintptr_t)ptr & (BUDDY_UNIT_SIZE - 1) != 0)
  // slab_free(ptr);
  // else
  buddy_free(&buddy_block[cpu_current() % 2], ptr);
}

#ifndef TEST
// 框架代码中的 pmm_init (在 AbstractMachine 中运行)
static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);

  // buddy_init(&buddy_block, heap.start, heap.end);

  buddy_init(&buddy_block[0], heap.start, heap.start + pmsize / 2);
  buddy_init(&buddy_block[1], heap.start + pmsize / 2, heap.end);
}
#else
Area heap;

// 测试代码的 pmm_init ()
static void pmm_init() {
  char *ptr = aligned_alloc(1 << 24, HEAP_SIZE);
  heap.start = ptr;
  heap.end = ptr + HEAP_SIZE;
  printf("Got %d MiB heap: [%p, %p)\n", HEAP_SIZE >> 20, heap.start, heap.end);

  // buddy_init(&buddy_block, heap.start, heap.end);
  // for (int i = 0; i < 2; i ++)

  buddy_init(&buddy_block[0], heap.start, heap.start + HEAP_SIZE / 2);
  buddy_init(&buddy_block[1], heap.start + HEAP_SIZE / 2, heap.end);
}
#endif

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
