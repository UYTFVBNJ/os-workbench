#include <buddy.h>

#define addr2idx(addr) \
  (((uintptr_t)addr - (uintptr_t)block->mem) >> block->UNIT_SHIFT)

void buddy_init(buddy_block_t *block, void *start, void *end) {
  // assign parameters

  block->TOTAL_SIZE =
      ((uintptr_t)end - (uintptr_t)start);  // TODO cal nearest 2^24
  assert(is_2_power(block->TOTAL_SIZE));
  assert(block->TOTAL_SIZE >= 1 << 24);
  block->TOTAL_SHIFT = num2shift(block->TOTAL_SIZE);

  block->UNIT_SHIFT = 12;                       // 4 KiB
  block->UNIT_SIZE = (1 << block->UNIT_SHIFT);  // 4 KiB
  block->UNIT_NUM = (block->TOTAL_SIZE / block->UNIT_SIZE);

  block->DS_NUM = (block->DS_SIZE / block->UNIT_SIZE);
  block->DS_SIZE = block->DS_NUM *
                   (sizeof(node_t) + sizeof(size_t));  // at least 4K * sizeof
  block->DS_UNIT_NUM = ((block->DS_SIZE - 1) / block->UNIT_SIZE) + 1;  // ceil

  // assign initial values
  block->lock.locked = 0;
  block->mem = start;
  block->bl_arr = start;
  block->fr_arr = start + block->DS_NUM * sizeof(node_t);
  assert(((uintptr_t)block->mem & 0xffffff) == 0);  // should align to 16MiB

#ifdef TEST
  printf("buddy: \n ");
  Log(block->TOTAL_SIZE);

  printf("\n");
  Log(block->UNIT_SHIFT);
  Log(block->UNIT_SIZE);
  Log(block->UNIT_NUM);

  printf("\n");
  Log(block->DS_NUM);
  Log(block->DS_SIZE);
  Log(block->DS_UNIT_NUM);
#endif

  block->bl_lst[block->TOTAL_SHIFT].front = &block->bl_arr[0];

  for (int i = 0; i < block->DS_NUM; i++)
    block->bl_arr[i] = (node_t){
        .key = block->mem + i * block->UNIT_SIZE,
    };

  // for (int i = 0; i < block->DS_NUM; i++) block->fr_arr[i] =
  // block->UNIT_SHIFT;

  buddy_alloc(block, block->DS_SIZE);
}

void *buddy_alloc(buddy_block_t *block, size_t size) {
  int sz_xft = is_2_power(size) ? num2shift(size) : num2shift(size) + 1;

  lock(&block->lock);

  int i;
  for (i = sz_xft; i <= block->TOTAL_SHIFT; i++) {
    if (block->bl_lst[i].front != NULL) {
      break;
    }
  }

  for (; i > sz_xft; i--) {
    node_t *bl_nd = block->bl_lst[i].front;

    list_pop_front(&block->bl_lst[i]);

    list_push_front(&block->bl_lst[i - 1],
                    &block->bl_arr[addr2idx(bl_nd->key)]);
    list_push_front(&block->bl_lst[i - 1], bl_nd);
  }

  node_t *bl_nd = block->bl_lst[sz_xft].front;
  block->fr_arr[addr2idx(bl_nd->key)] = sz_xft;
  list_pop_front(&block->bl_lst[sz_xft]);

  unlock(&block->lock);

  return bl_nd->key;
}

void buddy_free(buddy_block_t *block, void *ptr) {
  lock(&block->lock);

  int idx = addr2idx(ptr);
  int sz_xft = block->fr_arr[idx];

  node_t *bl_nd;
  for (bl_nd = block->bl_lst[sz_xft].front; bl_nd != NULL && bl_nd != ptr;
       bl_nd = bl_nd->nxt)
    ;  // take O(n) time. abort.
  assert(bl_nd != NULL);

  unlock(&block->lock);
}