#include <buddy.h>

#define addr2idx(addr)                                                         \
  (((uintptr_t)addr - (uintptr_t)block->mem) >> block->UNIT_SHIFT)

#define idx2addr(idx) (block->mem + (idx << block->UNIT_SHIFT))

#define buddy_idx(idx, sz_xft) (idx ^ (1 << (sz_xft - block->UNIT_SHIFT)))

inline bool
buddy_list_empty(list_t* list)
{
  return list->nil.nxt == &list->nil;
}

inline void
buddy_list_insert(list_t* list, node_t* node)
{
  node->nxt = list->nil.nxt;
  list->nil.nxt->pre = node;
  list->nil.nxt = node;
  node->pre = &list->nil;
}

inline void
buddy_list_delete(list_t* list, node_t* node)
{
  node->pre->nxt = node->nxt;
  node->nxt->pre = node->pre;
}

void
buddy_init(buddy_block_t* block, void* start, void* end)
{
  if (((uintptr_t)end & 0xffffff) != 0) {
    end = (void*)((uintptr_t)end & ~0xffffff); // should align to 16MiB
    printf("change end to %p", end);
  }

  if (((uintptr_t)start & (BUDDY_UNIT_SIZE - 1)) != 0) {
    start = (void*)(((uintptr_t)start & ~(BUDDY_UNIT_SIZE - 1)) +
                    BUDDY_UNIT_SIZE); // should align to 4KiB
    printf("change start to %p", start);
  }

  assert(((uintptr_t)end & 0xffffff) == 0); // should align to 16MiB

  void* buddy_start =
    (void*)((uintptr_t)end -
            (1 << ceil_shift((uintptr_t)end - (uintptr_t)start)));

  // assign parameters

  block->TOTAL_SIZE = ((uintptr_t)end - (uintptr_t)buddy_start);
  assert(is_2_power(block->TOTAL_SIZE));
  assert(block->TOTAL_SIZE >= 1 << 24);
  // assert(0);
  block->TOTAL_SHIFT = num2shift(block->TOTAL_SIZE);

  block->UNIT_SHIFT = BUDDY_UNIT_SHIFT;        // 4 KiB
  block->UNIT_SIZE = (1 << block->UNIT_SHIFT); // 4 KiB
  block->UNIT_NUM = (block->TOTAL_SIZE / block->UNIT_SIZE);

  block->DS_NUM = block->UNIT_NUM;
  block->DS_SIZE =
    block->DS_NUM *
    (sizeof(node_t) + sizeof(buddy_unit_ds_t)); // at least 4K * sizeof
  block->DS_UNIT_NUM = ((block->DS_SIZE - 1) / block->UNIT_SIZE) + 1; // ceil

  // assign initial values
  block->lock.locked = 0;
  block->mem = buddy_start;
  block->bl_arr = start;
  block->ds_arr = start + block->DS_NUM * sizeof(node_t);

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
      (node_t){ .key = block->ds_arr + i, .pre = NULL, .nxt = NULL };
    block->ds_arr[i].sz_xft = -1;
    block->ds_arr[i].belong = -1;
    block->ds_arr[i].idx = i;
  }

  for (int i = 0; i <= block->TOTAL_SHIFT; i++)
    block->bl_lst[i].nil.nxt = block->bl_lst[i].nil.pre = &block->bl_lst[i].nil;

  buddy_list_insert(&block->bl_lst[block->TOTAL_SHIFT], &block->bl_arr[0]);
  ((buddy_unit_ds_t*)(block->bl_arr[0].key))->belong = block->TOTAL_SHIFT;

  // for (int i = 0; i < block->DS_NUM; i++) block->fr_arr[i] =
  // block->UNIT_SHIFT;

  assert(buddy_alloc(
           block, block->DS_SIZE + (uintptr_t)start - (uintptr_t)buddy_start) ==
         buddy_start);

  printf("buddy initialized successfully\n area: [%p, %p)\n", buddy_start, end);
}

void*
buddy_alloc(buddy_block_t* block, size_t size)
{
  int sz_xft = ceil_shift(size);
  if (sz_xft < block->UNIT_SHIFT)
    sz_xft = block->UNIT_SHIFT;

  lock(&block->lock);

  int i;
  for (i = sz_xft; i <= block->TOTAL_SHIFT; i++) {
    if (!buddy_list_empty(&block->bl_lst[i])) {
      break;
    }
  }

  // no enough space
  if (i > block->TOTAL_SHIFT) {
    unlock(&block->lock);
    return NULL;
  }

  for (; i > sz_xft; i--) {
    node_t* bl_nd = block->bl_lst[i].nil.nxt;
    node_t* bl_nd_buddy =
      &block->bl_arr[buddy_idx(((buddy_unit_ds_t*)(bl_nd->key))->idx, i - 1)];

    buddy_list_delete(&block->bl_lst[i], bl_nd);

    buddy_list_insert(&block->bl_lst[i - 1], bl_nd_buddy);

    buddy_list_insert(&block->bl_lst[i - 1], bl_nd);

    // (buddy_unit_ds_t *)(bl_nd->key)->belong = i - 1;
    ((buddy_unit_ds_t*)(bl_nd_buddy->key))->belong = i - 1;
  }

  node_t* bl_nd = block->bl_lst[sz_xft].nil.nxt;
  ((buddy_unit_ds_t*)(bl_nd->key))->sz_xft = sz_xft;
  ((buddy_unit_ds_t*)(bl_nd->key))->belong = -1;
  buddy_list_delete(&block->bl_lst[sz_xft], bl_nd);

  // accessing buddy_block
  void* ret = idx2addr(((buddy_unit_ds_t*)(bl_nd->key))->idx);
  // check alignment
#ifdef TEST
  // assert(((uintptr_t)idx2addr(((buddy_unit_ds_t*)bl_nd->key)->idx) &
  // ((1 << sz_xft) - 1)) == 0);
#endif
  unlock(&block->lock);
  return ret;
}

void
buddy_free(buddy_block_t* block, void* ptr)
{
  lock(&block->lock);

  int idx = addr2idx(ptr);
  int sz_xft = block->ds_arr[idx].sz_xft;

  block->ds_arr[idx].sz_xft = -1;

  int i;
  for (i = sz_xft;
       i < block->TOTAL_SHIFT && block->ds_arr[buddy_idx(idx, i)].belong == i;
       i++) {
    buddy_list_delete(&block->bl_lst[i], &block->bl_arr[buddy_idx(idx, i)]);
    block->ds_arr[buddy_idx(idx, i)].belong = -1;

    idx = idx & buddy_idx(idx, i);
  }

  buddy_list_insert(&block->bl_lst[i], &block->bl_arr[idx]);
  block->ds_arr[idx].belong = i;

  unlock(&block->lock);
}

bool
buddy_check_alloced(buddy_block_t* block, void* ptr)
{
  lock(&block->lock);

  int idx = addr2idx(ptr);
  int sz_xft = block->ds_arr[idx].sz_xft;

  unlock(&block->lock);

  return (sz_xft == -1) ? false : true;
}