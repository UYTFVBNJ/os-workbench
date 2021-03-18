#include <game.h>

Time readtime() {
  AM_TIMER_UPTIME_T event;
  ioe_read(AM_TIMER_UPTIME, &event);
  return event.us / 1000;
}


