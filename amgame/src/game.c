#include <game.h>

// Operating system is a C program!
int main(const char *args) {
  ioe_init();

  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");

  uint32_t i = 0;
  while(1) splash_c(i++);

  puts("Press any key to see its key code...\n");
  while (1) {
    print_key();
  }
  return 0;
}
