#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <klib-macros.h>

void splash();
void splash_c(uint32_t);
void print_key();
static inline void puts(const char *s) {
  for (; *s; s++) putch(*s);
}
