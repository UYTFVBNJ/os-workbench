#include <assert.h>
#include <getopt.h>
#include <stdio.h>

int flag_p = 0, flag_n = 0, flag_V = 0;

int main(int argc, char* argv[]) {
  static struct option long_options[] = {{"show-pids", no_argument, 0, 'p'},
                                         {"numeric-sort", no_argument, 0, 'n'},
                                         {"version", no_argument, 0, 'V'},
                                         {0, 0, 0, 0}};

  int opt;
  while ((opt = getopt_long(argc, argv, "pnV", long_options, 0)) != -1) {
    switch (opt)
      case 'p':
        printf("p\n");
        break;
      case 'n':
        printf("n\n");
        break;

      case 'V':
        printf("V\n");
        break;

      default:
        break;
    }
  }

  // FILE fd = fopen("/proc/")
  return 0;
}
