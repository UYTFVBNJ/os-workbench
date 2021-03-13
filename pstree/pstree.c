#include <assert.h>
#include <getopt.h>
#include <stdio.h>

struct option long_options[] = {
  {"show-pids",    no_argument, 0, 'p'},
  {"numeric-sort", no_argument, 0, 'n'},
  {"version",      no_argument, 0, 'V'},
  {0,              0,           0,  0 }
};

int main(int argc, char* argv[]) {
  bool flag_p = false, flag_n = false, flag_V = false;
  int opt;
  while ((opt = getopt_long(argc, argv, "pnV")) != -1) {
    switch (opt) {
      case 'p':
        flag_p = true;
        printf("p\n");
        break;
      case 'n':
        flag_n = true;
        printf("n\n");
        break;

      case 'V':
        flag_V = true;
        printf("V\n");
        break;

      default:
        break;
    }
  }

  // FILE fd = fopen("/proc/")
  return 0;
}
