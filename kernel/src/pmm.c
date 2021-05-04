#include <buddy.h>
#include <pmm.h>
buddy_block_t buddy_block;

// hash
void *fast_path() { return NULL; }

// buddy
void *slow_path() { return NULL; }

// framework
static void *kalloc(size_t size) { return buddy_alloc(&buddy_block, size); }

static void kfree(void *ptr) { buddy_free(&buddy_block, ptr); }

#ifndef TEST
// 框架代码中的 pmm_init (在 AbstractMachine 中运行)
static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);

  buddy_init(&buddy_block, heap.start, heap.end);
}
#else
// 测试代码的 pmm_init ()
static void pmm_init() {
  char *ptr = aligned_alloc(1 << 24, HEAP_SIZE);
  heap.start = ptr;
  heap.end = ptr + HEAP_SIZE;
  printf("Got %d MiB heap: [%p, %p)\n", HEAP_SIZE >> 20, heap.start, heap.end);

  buddy_init(&buddy_block, heap.start, heap.end);
}
#endif

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
