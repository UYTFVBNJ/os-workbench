#include <assert.h>
#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define N 1000000
int flag_p = 1, flag_n = 1, flag_V = 0;

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
  struct Proc *v;
  struct Edge *nxt;
} * edges[N];

struct Edge *new_edge(struct Proc *v, struct Edge *nxt) {
  struct Edge *edge = (struct Edge *)malloc(sizeof(struct Edge));
  assert(edge);
  edge->v = v;
  edge->nxt = nxt;
  return edge;
}

int cnt = 0;

void get_thread_info(char * filename) {

  FILE *fd = fopen(filename, "r");
  assert(fd);

  char buf[64];
  int ret = fread(buf, 1, 64, fd);
  assert(ret == 64);

  int tmp_int, proc_pid = -1;
  char proc_state;

  sscanf(buf, "%d", &proc_pid);
  procs[proc_pid].pid = proc_pid;
  sscanf(buf, "%d %s %c %d", &tmp_int, procs[proc_pid].name, &proc_state,
         &procs[proc_pid].ppid);

  printf("%d %d\n %s\n %s \n", proc_pid, procs[proc_pid].ppid, buf,
         procs[proc_pid].name);

  edges[procs[proc_pid].ppid] =
          new_edge(&procs[proc_pid], edges[procs[proc_pid].ppid]);

  fclose(fd);

  cnt ++;
}

void get_proc_info() {
  DIR *d = opendir("/proc");
  assert(d);

  struct dirent *dir;
  while ((dir = readdir(d)) != NULL)
    if (dir->d_type == DT_DIR && is_num(dir->d_name)) {
      // printf("%s\n", dir->d_name);
      char pathname[256];

      int ret = snprintf(pathname, 256, "/proc/%s/task", dir->d_name);
      assert(ret >= 0);

      DIR * t_d = opendir(pathname);
      assert(t_d);

      struct dirent *t_dir;

      while ((t_dir = readdir(t_d)) != NULL) {
        ret = snprintf(pathname, 256, "/proc/%s/task/%s/stat", dir->d_name, t_dir->d_name);
        printf("%s\n", pathname);
        assert(ret >= 0);

        get_thread_info(pathname);
      }

      closedir(t_d);
    }

  closedir(d);
}

void print_tree(int u, int dep) {
  // for (int i = 0; i < 10000000; i++);
  // printf("%d:\n", u);
  cnt ++;

  for (int i = 0; i < dep; i++) printf("\t");

  printf("%s", procs[u].name);

  if (flag_p) {
    printf("(%d)\n", procs[u].pid);
  } else {
    printf("\n");
  }

  if (flag_n) {
    while (edges[u] != NULL) {
      struct Edge *ee =edges[u];
      for (struct Edge *e = edges[u]; e != NULL; e = e->nxt)
        if (e->v->pid < ee->v->pid) ee = e;

      print_tree(ee->v->pid, dep + 1);

      if (ee == edges[u]) {
        edges[u] = ee->nxt;
      } else
      for (struct Edge *e = edges[u]; e != NULL; e = e->nxt)
        if (e->nxt == ee) {
          e->nxt = ee->nxt;
          free(ee);
          break;
        }
    }
  } else {
    for (struct Edge *e = edges[u]; e != NULL; e = e->nxt)
      print_tree(e->v->pid, dep + 1);
  }
}

void input(int argc, char *argv[]) {
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

        exit(0);
        break;

      default:
        break;
    }
  }
}

int main(int argc, char *argv[]) {
  input(argc, argv);

  printf("flags: %d %d %d\n", flag_p, flag_n, flag_V);

  cnt = 0;
  get_proc_info();
  printf("%d\n", cnt);
  cnt = 0;
  print_tree(1, 0);
  printf("%d\n", cnt);

  return 0;
}
