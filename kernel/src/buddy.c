#include <buddy.h>

#define addr2idx(addr) \
  (((uintptr_t)addr - (uintptr_t)block->mem) >> block->UNIT_SHIFT)

#define idx2addr(idx) ((uint8_t *)block->mem + (idx << block->UNIT_SHIFT))

#define buddy_idx(idx, sz_xft) (idx ^ (1 << (sz_xft - block->UNIT_SHIFT)))

static bool initialing;

void buddy_init(buddy_block_t *block, void *start, void *end) {
  // assign parameters

  block->TOTAL_SIZE =
      ((uintptr_t)end - (uintptr_t)start);  // TODO cal nearest 2^24
  assert(is_2_power(block->TOTAL_SIZE));
  // assert(block->TOTAL_SIZE >= 1 << 24);
  block->TOTAL_SHIFT = num2shift(block->TOTAL_SIZE);

  block->UNIT_SHIFT = BUDDY_UNIT_SHIFT;         // 4 KiB
  block->UNIT_SIZE = (1 << block->UNIT_SHIFT);  // 4 KiB
  block->UNIT_NUM = (block->TOTAL_SIZE / block->UNIT_SIZE);

  block->DS_NUM = block->UNIT_NUM;
  block->DS_SIZE =
      block->DS_NUM *
      (sizeof(node_t) + sizeof(buddy_unit_ds_t));  // at least 4K * sizeof
  block->DS_UNIT_NUM = ((block->DS_SIZE - 1) / block->UNIT_SIZE) + 1;  // ceil

  // assign initial values
  block->lock.locked = 0;
  block->mem = start;
  block->bl_arr = start;
  block->ds_arr = start + block->DS_NUM * sizeof(node_t);
  assert(((uintptr_t)block->mem & 0xffffff) == 0);  // should align to 16MiB

#ifdef TEST
  printf("buddy: \n");
  printf("[%p, %p]: \n", start, end);
  Log(block->TOTAL_SIZE);

  printf("\n");
  Log(block->UNIT_SHIFT);
  Log(block->UNIT_SIZE);
  Log(block->UNIT_NUM);

  printf("\n");
  Log(block->DS_NUM);
  Log(block->DS_SIZE);
  Log(block->DS_UNIT_NUM);
  printf("%lf\n", (double)block->DS_UNIT_NUM / block->UNIT_NUM);
#endif

  for (int i = 0; i < block->DS_NUM; i++) {
    block->bl_arr[i] =
        (node_t){.key = block->ds_arr + i, .pre = NULL, .nxt = NULL};
    block->ds_arr[i].sz_xft = -1;
    block->ds_arr[i].belong = -1;
    block->ds_arr[i].idx = i;
  }

  for (int i = 0; i <= block->TOTAL_SHIFT; i++)
    block->bl_lst[i].nil.nxt = block->bl_lst[i].nil.pre = &block->bl_lst[i].nil;

  list_insert(&block->bl_lst[block->TOTAL_SHIFT], &block->bl_arr[0]);
  ((buddy_unit_ds_t *)(block->bl_arr[0].key))->belong = block->TOTAL_SHIFT;

  // for (int i = 0; i < block->DS_NUM; i++) block->fr_arr[i] =
  // block->UNIT_SHIFT;

  initialing = 1;
  assert(buddy_alloc(block, block->DS_SIZE) == block->mem);
  initialing = 0;
}

void *buddy_alloc(buddy_block_t *block, size_t size) {
  int sz_xft = is_2_power(size) ? num2shift(size) : num2shift(size) + 1;
  // TODO
  if (sz_xft < block->UNIT_SHIFT) sz_xft = block->UNIT_SHIFT;
  // assert(sz_xft >= block->UNIT_SHIFT);

  lock(&block->lock);

  int i;
  for (i = sz_xft; i <= block->TOTAL_SHIFT; i++) {
    if (!list_empty(&block->bl_lst[i])) {
      break;
    }
  }

  // no enough space
  if (i > block->TOTAL_SHIFT) {
    unlock(&block->lock);
    return NULL;
  }

  for (; i > sz_xft; i--) {
    node_t *bl_nd = block->bl_lst[i].nil.nxt;
    node_t *bl_nd_buddy =
        &block
             ->bl_arr[buddy_idx(((buddy_unit_ds_t *)(bl_nd->key))->idx, i - 1)];

    list_delete(&block->bl_lst[i], bl_nd);

    list_insert(&block->bl_lst[i - 1], bl_nd_buddy);

    list_insert(&block->bl_lst[i - 1], bl_nd);

    // (buddy_unit_ds_t *)(bl_nd->key)->belong = i - 1;
    ((buddy_unit_ds_t *)(bl_nd_buddy->key))->belong = i - 1;
  }

  node_t *bl_nd = block->bl_lst[sz_xft].nil.nxt;
  ((buddy_unit_ds_t *)(bl_nd->key))->sz_xft = sz_xft;
  ((buddy_unit_ds_t *)(bl_nd->key))->belong = -1;
  list_delete(&block->bl_lst[sz_xft], bl_nd);

  void *ret = idx2addr(((buddy_unit_ds_t *)bl_nd->key)->idx);

  unlock(&block->lock);

  // check alignment
  assert(((uintptr_t)idx2addr(((buddy_unit_ds_t *)bl_nd->key)->idx) &
          ((1 << sz_xft) - 1)) == 0);

#ifdef TEST
  if (!initialing) {
    uint32_t *addr = idx2addr(((buddy_unit_ds_t *)bl_nd->key)->idx);
    for (uint32_t *chk_ptr = addr; chk_ptr < addr + (1 << (sz_xft - 2));
         chk_ptr++) {
      if (*(uint32_t *)chk_ptr == USED(sz_xft)) printf("%p\n", chk_ptr);
      assert(*(uint32_t *)chk_ptr != USED(sz_xft));
      *(uint32_t *)chk_ptr = USED(sz_xft);
    }
  }
#endif

  return ret;
}

void buddy_free(buddy_block_t *block, void *ptr) {
  lock(&block->lock);

  int idx = addr2idx(ptr);
  int sz_xft = block->ds_arr[idx].sz_xft;

#ifdef TEST
  uint32_t *addr = ptr;
  for (uint32_t *chk_ptr = addr; chk_ptr < addr + (1 << (sz_xft - 2));
       chk_ptr++) {
    if (*(uint32_t *)chk_ptr != USED(sz_xft)) printf("%p\n", chk_ptr);
    assert(*(uint32_t *)chk_ptr == USED(sz_xft));
    *(uint32_t *)chk_ptr = 0;
  }
#endif

  block->ds_arr[idx].sz_xft = -1;

  int i;
  for (i = sz_xft;
       i < block->TOTAL_SHIFT && block->ds_arr[buddy_idx(idx, i)].belong == i;
       i++) {
    list_delete(&block->bl_lst[i], &block->bl_arr[buddy_idx(idx, i)]);
    block->ds_arr[buddy_idx(idx, i)].belong = -1;

    idx = idx & buddy_idx(idx, i);
  }

  list_insert(&block->bl_lst[i], &block->bl_arr[idx]);
  block->ds_arr[idx].belong = i;

  unlock(&block->lock);
}