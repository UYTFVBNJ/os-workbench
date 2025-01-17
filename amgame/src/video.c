#include <game.h>

extern Obj *objs[OBJS_MAX_NUM];
extern int objs_num;

#define SIDE 16
static int w, h;

static void init() {
  AM_GPU_CONFIG_T info = {0};
  ioe_read(AM_GPU_CONFIG, &info);
  w = info.width;
  h = info.height;
}

/* The draw family */

static void draw(int x, int y, int w, int h, Color color,
                 bool (*is_draw)(int, int, int)) {
  uint32_t pixels[w * h];  // WARNING: large stack-allocated memory
  AM_GPU_FBDRAW_T event = {
      .x = x,
      .y = y,
      .w = w,
      .h = h,
      .sync = 1,
      .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++)
    if (is_draw(w, h, i)) {
      pixels[i] = color;
    }
  ioe_write(AM_GPU_FBDRAW, &event);
}

void obj_draw(Obj const *obj) {
  draw(obj->x, obj->y, obj->w, obj->h, obj->color, obj->is_draw);
}

void obj_hide(Obj const *obj) {
  draw(obj->x, obj->y, obj->w, obj->h, 0x000000, obj->is_draw);
}

bool is_draw_rect(int obj_w, int obj_h, int idx) { return 1; }

bool is_draw_circ(int obj_w, int obj_h, int idx) {
  // int x = idx /
  // int r = min(obj_w, obj_h) / 2;
  return 1;
}

/* The screen family */
void screen_update() {
  for (int i = 0; i < objs_num; i++)
    // if (objs[i]->upd) {
    obj_draw(objs[i]);
  // objs[i]->upd = false;
  // }
}

void screen_init() {
  init();
  printf("screen size: %d %d\n", w, h);
}
