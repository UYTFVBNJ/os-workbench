#ifndef BUDDY
#define BUDDY

#include <minilib.h>
#include <pmm.h>
#include <nv_spinlock.h>

typedef struct
{
  char sz_xft;
  char belong;
  int idx;
} buddy_unit_ds_t;

typedef struct
{
  nv_spinlock_t lock;

  uint8_t* mem;
  intptr_t start;
  list_t bl_lst[32];

  node_t* bl_arr;
  buddy_unit_ds_t* ds_arr;

  size_t TOTAL_SIZE, TOTAL_SHIFT;
  size_t UNIT_SHIFT, UNIT_SIZE, UNIT_NUM;
  size_t DS_NUM, DS_SIZE, DS_UNIT_NUM;
} buddy_block_t;

void
buddy_init(buddy_block_t* block, void* start, void* end);
void*
buddy_alloc(buddy_block_t* block, size_t size);
void
buddy_free(buddy_block_t* block, void* ptr);
bool
buddy_check_alloced(buddy_block_t* block, void* ptr);

#endif