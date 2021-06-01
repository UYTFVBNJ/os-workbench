#ifndef MINILIB
#define MINILIB

#include <common.h>
#include <nv_spinlock.h>

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

#define Log(num) printf("  %s: %d\n", #num, num);

typedef struct node
{
  struct node *pre, *nxt;
  void* key;
} node_t;

typedef struct list
{
  nv_spinlock_t lock;
  node_t nil;
} list_t;

bool
list_empty(list_t* list);
void
list_insert(list_t* list, node_t* node);
void
list_insert_at(list_t* list, node_t* node1, node_t* node2);
void
list_delete(list_t* list, node_t* node);

bool
is_2_power(int64_t n);

int
ceil_shift(int64_t n);

int
num2shift(int64_t n);

uint64_t
uptime();

#endif