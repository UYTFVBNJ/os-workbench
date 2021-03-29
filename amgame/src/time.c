#include <game.h>

Time uptime() {
  AM_TIMER_UPTIME_T event;
  ioe_read(AM_TIMER_UPTIME, &event);
  return event.us / 1000;
}


