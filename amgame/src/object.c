#include <game.h>

#define S(x, n) (x[(n + 1) % 4])
#define P(x, n) (x[(n + 3) % 4])

extern Obj *objs[OBJS_MAX_NUM];
extern int objs_num;

Obj *obj_creat(int type, int x, int y, int w, int h, Color color,

               bool (*is_draw)(int, int, int), void (*collision_handler)(void *, int, int)) {
  if (objs_num == sizeof(objs) / sizeof(Obj *)) return NULL;

  objs[objs_num] = (Obj *)malloc(sizeof(Obj));

  // objs[objs_num]->upd = true;

  objs[objs_num]->type = type;

  objs[objs_num]->idx = objs_num;

  objs[objs_num]->v_x = 0;
  objs[objs_num]->v_y = 0;

  objs[objs_num]->x = x;
  objs[objs_num]->y = y;

  objs[objs_num]->w = w;
  objs[objs_num]->h = h;

  objs[objs_num]->color = color;

  objs[objs_num]->is_draw = is_draw;
  objs[objs_num]->collision_handler = collision_handler;

  objs_num++;

  return objs[objs_num - 1];
}

void obj_remove(Obj *obj) {
  obj_hide(obj);

  int idx = obj->idx;

  free(objs[idx]);

  for (int i = idx; i < objs_num; i++) {
    objs[i] = objs[i + 1];
  }
  // last iteration objs[pre_objs_num - 1] = NULL;

  objs_num--;
}

void obj_move(Obj *obj) {
  obj_hide(obj);

  obj->x += obj->v_x;
  obj->y += obj->v_y;
  // obj->upd = true;

  // TODO
}

void board_collision_handler(Obj *obj, int side, int type) {}

void ball_collision_handler(Obj *obj, int side, int type) {
  switch (side) {
    case 1:
    case 3:
      obj->v_y = -obj->v_y;
      break;
    case 2:
    case 4:
      obj->v_x = -obj->v_x;
      break;
    default:
      assert(0);
  }
}

void brick_collision_handler(Obj *obj, int side, int type) { obj_remove(obj); }

bool line_collision_detector(int a_x0, int a_x1, int a_y, int b_x0, int b_x1,
                             int b_y, int av_x, int av_y) {
  a_x0 *= OBJS_MAP_SCALE;
  a_x1 *= OBJS_MAP_SCALE;
  a_y *= OBJS_MAP_SCALE;

  b_x0 *= OBJS_MAP_SCALE;
  b_x1 *= OBJS_MAP_SCALE;
  b_y *= OBJS_MAP_SCALE;

  if (av_y == 0) return 0;

  int t = (b_y - a_y) / av_y;

  printf("%d %d %d %d %d %d %d\n", a_x0 / OBJS_MAP_SCALE, a_x1 / OBJS_MAP_SCALE, a_y / OBJS_MAP_SCALE, b_x0 / OBJS_MAP_SCALE, b_x1 / OBJS_MAP_SCALE, b_y / OBJS_MAP_SCALE, t / OBJS_MAP_SCALE);
  if (t < 0) return 0;

  a_x0 += t * av_x;
  a_x1 += t * av_x;

  if (a_x1 < b_x0 || b_x1 < a_x0) return 0;

  if (t > OBJS_MAP_SCALE) return 0;

  return 1;
}

int obj_collision_detector(Obj const *a, Obj const *b) {
  /* assume that b is still */
  int A[] = {a->y, a->x + a->w, a->y + a->h, a->x};

  int B[] = {b->y, b->x + b->w, b->y + b->h, b->x};
  int av_x = a->v_x, av_y = a->v_y;

  for (int i = 0; i < 4; i++) {
    if (line_collision_detector(P(A, i), S(A, i), A[i], S(B, i + 2),
                                P(B, i + 2), B[(i + 2) % 4], av_x, av_y))
      return i + 1;
    
    // swap(av_x, av_y);
    int tmp = av_x;
    av_x = av_y;
    av_y = tmp;
  }
  return 0;
}