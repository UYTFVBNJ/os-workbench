#include <am.h>
#include <amdev.h>
#include <klib-macros.h>
#include <klib.h>

typedef int Key;
typedef uint32_t Time;
typedef uint32_t Color;

struct Object {
  bool upd;
  int type;
  int spd_v, spd_h;
  int x, y;
  int w, h;
  Color color;
  bool (*is_draw)(int, int, int);
};
typedef struct Object Obj;

/* video */

void obj_draw(Obj *);
bool is_draw_rect(int, int, int);
bool is_draw_circ(int, int, int);

void screen_init();
void screen_update();

/* keyboard */
void print_key();
static inline void puts(const char *s) {
  for (; *s; s++) putch(*s);
}

Key readkey();

/* time */
Time readtime();