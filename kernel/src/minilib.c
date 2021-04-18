#include <minilib.h>

// list_t
void list_push_front(list_t *list, node_t *node) {
  spinlock(&list->lock);

  node->pre = NULL;
  node->nxt = list->front;
  list->front = node;

  spinunlock(&list->lock);
}

void list_pop_front(list_t *list) {
  spinlock(&list->lock);

  assert(list->front != NULL);

  if (list->front->nxt != NULL) list->front->nxt->pre = NULL;

  list->front = list->front->nxt;

  spinunlock(&list->lock);
}

bool is_2_power(int64_t n) {
  for (int64_t i = 1; i <= INT64_MAX; i <<= 1)
    if (i == n) return true;
  return false;
}

int num2shift(int64_t n) {
  int cnt = 0;
  while (n >>= 1) cnt++;
  return cnt;
}
