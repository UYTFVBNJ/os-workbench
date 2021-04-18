#include <common.h>
#include <minilib.h>
#include <spinlock.h>

typedef struct {
  spinlock_t lock;

  uint8_t *mem;
  list_t bl_lst[32];

  node_t *bl_arr;
  size_t *fr_arr;

  size_t TOTAL_SIZE, TOTAL_SHIFT;
  size_t UNIT_SHIFT, UNIT_SIZE, UNIT_NUM;
  size_t DS_NUM, DS_SIZE, DS_UNIT_NUM;
} buddy_block_t;
