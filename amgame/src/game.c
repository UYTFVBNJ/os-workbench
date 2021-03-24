#include <game.h>

const Time FPS = 30;

enum { OBJ_BOARD, OBJ_BALL, OBJ_BRICK };

Obj *objs[OBJS_MAX_NUM];
int objs_num = 0;
static Obj *ball, *board;

void game_init() {
  ball = obj_creat(OBJ_BALL, 100, 50, 20, 20, 0xffffff, is_draw_rect, ball_collision_handler);
  ball->v_y = 1;

  board = obj_creat(OBJ_BOARD, 50, 100, 40, 20, 0xffffff, is_draw_rect, board_collision_handler);

  obj_creat(OBJ_BRICK, 50, 10, 40, 20, 0xffffff, is_draw_rect, brick_collision_handler);

  screen_init();
}

void kbd_event(Key key) {
  switch (key) {
    case AM_KEY_LEFT:
      board->v_y -= 2;
      break;
    case AM_KEY_RIGHT:
      board->v_y += 2;
      break;
    case AM_KEY_ESCAPE:
      halt(0);
      break;

    default:
      break;
  }
}

void game_collision_handler() {
  /* ball vs. board & brick*/
  int ret;
  for (int i = 0; i < objs_num; i++)
    if (objs[i] != ball)
      if ((ret = obj_collision_detector(ball, objs[i])) > 0) {
        ball->collision_handler(ball, ret);
        objs[i]->collision_handler(objs[i], (ret + 2) % 4);
      }
}

void game_movement_handler() {
  for (int i = 0; i < objs_num; i++)
    if (objs[i]->v_x != 0 || objs[i]->v_y != 0) {
      obj_move(objs[i]);
      printf("%d %d\n", i, objs_num);
    }
}

void game_progress() {
  game_collision_handler();
  game_movement_handler();

  screen_update();
}

void game_loop() {
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

  /* begin */
  game_init();
  game_loop();

  return 0;
}
