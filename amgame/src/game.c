#include <game.h>

const Time FPS = 30;

enum { OBJ_BOARD, OBJ_BALL, OBJ_BRICK };

Obj *objs[10];
int objs_num = 0;
static Obj *ball, *board;

Obj *obj_creat(int type, int x, int y, int w, int h, Color color,
               bool (*is_draw)(int, int, int)) {
  if (objs_num == sizeof(objs) / sizeof(Obj *)) return NULL;

  objs[objs_num] = (Obj *)malloc(sizeof(Obj));

  objs[objs_num]->upd = true;

  objs[objs_num]->type = type;

  objs[objs_num]->spd_v = 0;
  objs[objs_num]->spd_h = 0;

  objs[objs_num]->x = x;
  objs[objs_num]->y = y;

  objs[objs_num]->w = w;
  objs[objs_num]->h = h;

  objs[objs_num]->color = color;

  objs[objs_num]->is_draw = is_draw;

  objs_num++;

  return objs[objs_num - 1];
}

void obj_remove(int idx) {
  free(objs[idx]);

  for (int i = idx; i < objs_num; i++) {
    objs[i] = objs[i + 1];
  }
  // last iteration objs[pre_objs_num - 1] = NULL;

  objs_num--;
}

void obj_move(Obj * obj) {
  obj_hide(obj);

  obj->x += obj->spd_h;
  obj->y += obj->spd_v;

  obj_draw(obj);
}

void game_init() {
  ball = obj_creat(OBJ_BALL, 50, 50, 20, 20, 0xffffff, is_draw_rect);
  ball->spd_v = -10;

  board = obj_creat(OBJ_BOARD, 50, 100, 40, 20, 0xffffff, is_draw_rect);

  obj_creat(OBJ_BRICK, 50, 10, 40, 20, 0xffffff, is_draw_rect);

  printf("qwq\n");
  screen_init();
  printf("qwq\n");
}

void kbd_event(Key key) {
  switch (key) {
    case AM_KEY_LEFT:
      board->spd_h -= 2;
      break;
    case AM_KEY_RIGHT:
      board->spd_h += 2;
      break;
    case AM_KEY_ESCAPE:
      halt(0);
      break;

    default:
      break;
  }
}

bool game_collision_detection(Obj *a, Obj *b) {
  // TODO
  return 0;
}

void game_collision_detection_handler(Obj *a) {
  // TODO
}

void game_progress() {
  /* ball vs. board & brick*/
  for (int i = 0; i < objs_num; i++)
    if (objs[i] != ball)
      if (game_collision_detection(ball, objs[i])) {
        game_collision_detection_handler(ball);
        game_collision_detection_handler(objs[i]);
      }

  for (int i = 0; i < objs_num; i++) if (objs[i]->spd_h != 0 || objs[i]->spd_v != 0) {
    obj_move(objs[i]);
  }
}

void game_loop() {
  while (1);
  Time next_frame = 0;
  while (1) {
    while (uptime() < next_frame)
      ;  // 等待一帧的到来
    Key key;
    while ((key = readkey()) != AM_KEY_NONE) {
      kbd_event(key);  // 处理键盘事件
    }

    game_progress();  // 处理一帧游戏逻辑，更新物体的位置等
    next_frame += 1000 / FPS;  // 计算下一帧的时间
  }
}

// Operating system is a C program!
int main(const char *args) {
  ioe_init();

  /*
  puts("mainargs = \"");
  puts(args);  // make run mainargs=xxx
  puts("\"\n");

  // uint32_t i = 0;
  // while (1) splash_c(i++);

  puts("Press any key to see its key code...\n");
  while (1) {
    print_key();
  }
  */

  /* begin */
  game_init();
  game_loop();

  return 0;
}
