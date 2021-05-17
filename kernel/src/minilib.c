#include <minilib.h>

// list_t
// TODO lock outside
bool
list_empty(list_t* list)
{
  return list->nil.nxt == &list->nil;
}

void
list_insert(list_t* list, node_t* node)
{
  node->nxt = list->nil.nxt;
  list->nil.nxt->pre = node;
  list->nil.nxt = node;
  node->pre = &list->nil;
}

void
list_insert_at(list_t* list, node_t* node1, node_t* node2)
{
  // TODO
}

void
list_delete(list_t* list, node_t* node)
{
  node->pre->nxt = node->nxt;
  node->nxt->pre = node->pre;
}

bool
is_2_power(int64_t n)
{
  for (int64_t i = 1; i <= INT64_MAX; i <<= 1)
    if (i == n)
      return true;
  return false;
}

int
num2shift(int64_t n)
{
  int cnt = 0;
  while (n >>= 1)
    cnt++;
  return cnt;
}

int
ceil_shift(int64_t n)
{
  int64_t m = 1;
  for (int i = 0; m <= INT64_MAX; m <<= 1, i++)
    if (m >= n)
      return i;
  return -1;
}

uint64_t
uptime()
{
  AM_TIMER_UPTIME_T event;
  ioe_read(AM_TIMER_UPTIME, &event);
  return event.us;
}