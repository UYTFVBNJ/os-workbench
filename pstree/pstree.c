#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <dirent.h>

int flag_p = 0, flag_n = 0, flag_V = 0;

int main(int argc, char* argv[]) {
  static struct option long_options[] = {
      {"show-pids", no_argument, NULL, 'p'},
      {"numeric-sort", no_argument, NULL, 'n'},
      {"version", no_argument, NULL, 'V'},
      {0, 0, 0, 0}};

  int opt = 0;
  while ((opt = getopt_long(argc, argv, "pnV", long_options, 0)) != -1) {
    switch (opt) {
      case 'p':
        flag_p = 1;
        printf("p\n");
        break;
      case 'n':
        flag_n = 1;
        printf("n\n");
        break;

      case 'V':
        flag_V = 1;
        printf("V\n");
        break;

      default:
        break;
    }
  }

  printf("flags: %d %d %d\n", flag_p, flag_n, flag_V);

  DIR *d = opendir("/proc");
  assert(d);

  struct dirent *dir;
  while ((dir = readdir(d)) != NULL) {
    printf("%s\n", dir->d_name);
  }

  // FILE fd = fopen("/proc/")
  return 0;
}
