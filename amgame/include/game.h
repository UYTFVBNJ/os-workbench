#include <am.h>
#include <amdev.h>
#include <klib-macros.h>
#include <klib.h>

typedef int Key;
typedef uint64_t Time;
typedef uint32_t Color;

/* object */
enum { OBJ_BOARD, OBJ_BALL, OBJ_BRICK };

#define OBJS_MAX_NUM 40
#define OBJS_MAP_SCALE (1 << 20)

struct Object {
  // bool upd;
  int type;
  int idx;
  int v_x, v_y;
  int x, y;
  int w, h;
  Color color;
  bool (*is_draw)(int, int, int);
  void (*collision_handler)(void *, int, int);
};
typedef struct Object Obj;

/* video */

bool is_draw_rect(int, int, int);
bool is_draw_circ(int, int, int);

void obj_draw(Obj const *obj);
void obj_hide(Obj const *obj);

void screen_init();
void screen_update();

/* keyboard */
void print_key();
static inline void puts(const char *s) {
  for (; *s; s++) putch(*s);
}

Key readkey();

/* time */
Time uptime();

/* object */
Obj *obj_creat(int type, int x, int y, int w, int h, Color color,
               bool (*is_draw)(int, int, int), void (*collision_handler)());

void obj_move(Obj *obj);
void obj_remove(Obj *obj);

void board_collision_handler(Obj *obj, int side, int type);
void ball_collision_handler(Obj *obj, int side, int type);
void brick_collision_handler(Obj *obj, int side, int type);

int obj_collision_detector(Obj const *a, Obj const *b);