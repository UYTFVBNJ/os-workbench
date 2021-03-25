#include <game.h>

const Time FPS = 30;

bool game_continue = 0;

Obj *objs[OBJS_MAX_NUM];
int objs_num = 0;
static Obj *ball, *board;

void game_init() {
  while (objs_num--) {
    obj_remove(objs[objs_num]);
  }

  objs_num = 0;

  game_continue = 1;

  ball = obj_creat(OBJ_BALL, 50, 250, 10, 10, 0xffffff, is_draw_rect, ball_collision_handler);
  ball->v_y = 1;

  board = obj_creat(OBJ_BOARD, 50, 290, 40, 10, 0xffffff, is_draw_rect, board_collision_handler);

  obj_creat(OBJ_BRICK, 5,   10, 40, 20, 0xffffff, is_draw_rect, brick_collision_handler);
  obj_creat(OBJ_BRICK, 55,  10, 40, 20, 0xffffff, is_draw_rect, brick_collision_handler);
  obj_creat(OBJ_BRICK, 105, 10, 40, 20, 0xffffff, is_draw_rect, brick_collision_handler);
  obj_creat(OBJ_BRICK, 155, 10, 40, 20, 0xffffff, is_draw_rect, brick_collision_handler);
  obj_creat(OBJ_BRICK, 205, 10, 40, 20, 0xffffff, is_draw_rect, brick_collision_handler);
  obj_creat(OBJ_BRICK, 255, 10, 40, 20, 0xffffff, is_draw_rect, brick_collision_handler);
  obj_creat(OBJ_BRICK, 305, 10, 40, 20, 0xffffff, is_draw_rect, brick_collision_handler);
  obj_creat(OBJ_BRICK, 355, 10, 40, 20, 0xffffff, is_draw_rect, brick_collision_handler);

  screen_update();
}

void game_win() {
  game_continue = 0;
  puts("game_win");
}

void game_over() {
  game_continue = 0;
  puts("game_over");
}

void kbd_event(Key key) {
  switch (key) {
    case AM_KEY_LEFT:
      board->v_x -= 1;
      break;
    case AM_KEY_RIGHT:
      board->v_x += 1;
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
        ball->collision_handler(ball, ret, objs[i]->type);
        objs[i]->collision_handler(objs[i], (ret + 2) % 4, ball->type);
        if (objs[i] == board) {
          ball->v_x += board->v_x;
        }
      }

  /* wall vs. ball & board */
  
  if (ball->x + ball->v_x <= 0) {
    ball->v_x = -ball->v_x;
  }
  
  if (ball->x + ball->w + ball->v_x >= 400) {
    ball->v_x = -ball->v_x;
  }

  if (ball->y <= 0) {
    ball->v_y = -ball->v_y;
  }
  
  if (ball->y + ball->h + ball->v_y >= 300) {
    board->v_y = 0;
    game_over();
  }

  if (board->x + board->v_x <= 0) {
    board->v_x = 0;
  }
  
  if (board->x + board->w + board->v_x>= 400) {
    board->v_x = 0;
  }
  
}

void game_movement_handler() {
  for (int i = 0; i < objs_num; i++)
    if (objs[i]->v_x != 0 || objs[i]->v_y != 0) {
      obj_move(objs[i]);
      // printf("%d %d\n", i, objs_num);
    }
}

void game_progress() {
  game_collision_handler();
  game_movement_handler();

  screen_update();
}

void game_loop() {
  Time next_frame = 0;
  while (game_continue) {
    while (uptime() < next_frame)
      ;  // 等待一帧的到来
    Key key;
    while ((key = readkey()) != AM_KEY_NONE) {
      kbd_event(key);  // 处理键盘事件
    }

    game_progress();  // 处理一帧游戏逻辑，更新物体的位置等
    next_frame += 1000 / FPS;  // 计算下一帧的时间
    if (uptime() > next_frame) printf("timeout\n");
  }
}

// Operating system is a C program!
int main(const char *args) {
  ioe_init();

  screen_init();

  /* begin */
  while (1) {
    game_init();
    game_loop();
  }

  return 0;
}
