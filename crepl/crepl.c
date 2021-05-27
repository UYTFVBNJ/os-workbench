#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

int line_num = 0;

int main(int argc, char *argv[]) {
  static char line[4096];
  while (1) {
    line_num ++;
    printf("crepl> ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    printf("line: %s\n", line);
    // if (strstr(line, "int") == line) func_ld(line);
    // else 
    expr_hdl(line);
    printf("Got %zu chars.\n", strlen(line)); // ??
  }
}

// void func_hdl(char *s) {
  // char filename[128];
  // sprintf(filename, "expr_%d_XXXXXX", line_num);
  // int fd = mkstemp(filename);
// }

void expr_hdl(char *s) {
  char func_name[128];
  sprintf(func_name, "expr_%d", line_num);

  char c_src[256];
  sprintf(c_src, "int %s() { return %s; }", func_name, s);

  void *handle = load(func_name, c_src);
  assert(handle != NULL);

  int (* expr)() = dlsym(handle, func_name);
  printf("= %d\n", expr());
}

void* load(char *func_name, char *c_src) {
  char file_path[128];
  sprintf(file_path, "/tmp/%s_XXXXXX", func_name);

  char so_path[128];
  sprintf(so_path, "%s.so", file_path);

  int fd = mkstemp(file_path);
  assert(fd != -1);

  write(fd, file_path, strlen(file_path));

  // gcc
  pid_t pid = fork();
  assert(pid != -1);

  if (pid == 0) {
    // child 
    execlp("gcc", "-shared", file_path, "-o", so_path);
  } else {
    // parent
    wait(-1);
    return dlopen(so_path, RTLD_LAZY);
  }
}