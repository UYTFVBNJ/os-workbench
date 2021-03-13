#include <assert.h>
#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define N 1000000
int flag_p = 0, flag_n = 0, flag_V = 0;

int is_num(char *str) {
  for (int i = 0; str[i] != '\0'; i++)
    if (str[i] < '0' || str[i] > '9') return 0;

  return 1;
}

struct Proc {
  pid_t pid;
  pid_t ppid;
  char name[256];
  // struct edge *edges;
} procs[N];

struct Edge {
  struct Proc v;
  struct Edge *nxt;
} *edges[N];

struct Edge *new_edge(struct Proc *v, struct Edge *nxt) {
  struct Edge *edge = (struct Edge *)malloc(sizeof(Edge));
  assert(edge);
  edge->v = v;
  edge->nxt = nxt;
  return edge;
}

void get_procinfo() {
  DIR *d = opendir("/proc");
  assert(d);

  struct dirent *dir;
  while ((dir = readdir(d)) != NULL)
    if (dir->d_type == DT_DIR && is_num(dir->d_name)) {
      printf("%s\n", dir->d_name);
      char filename[256];
      int ret;
      ret = snprintf(filename, 256, "/proc/%s/stat", dir->d_name);
      assert(ret >= 0);

      printf("%s\n", filename);
      FILE *fd = fopen(filename, "r");
      assert(fd);

      char buf[64];
      ret = fread(buf, 1, 64, fd);
      assert(ret == 64);

      int tmp_int, proc_pid;
      char proc_state;

      sscanf("%d", buf, proc_pid);
      sscanf("%d %s %c %d %d", buf, NULL, procs[proc_pid].name, &proc_state,
             procs[proc_pid].ppid);

      if (procs[proc_pid].ppid != -1) {
        edges[procs[proc_pid].ppid] =
            new_edge(&procs[proc_pid], edges[procs[proc_pid].ppid]);
      }

      fclose(fd);
    }
}

void print_tree(int u, int dep) {
  for (int i = 0; i < dep; i++) printf("\t");
  printf("%s ", procs[u].name);
  for (struct Edge *e = edges[u]; e != NULL; e = e->nxt)
    print_tree(e->v->pid, dep + 1);
}

int main(int argc, char *argv[]) {
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

        //

        return 0;
        break;

      default:
        break;
    }
  }

  printf("flags: %d %d %d\n", flag_p, flag_n, flag_V);

  get_procinfo();

  print_tree(1, 0);

  return 0;
}
