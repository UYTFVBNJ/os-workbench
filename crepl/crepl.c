#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dlfcn.h>

#define SZ_BUF 512
int line_num = 0;

void* load(char *func_name, char *c_src);
void func_hdl(char *s);
void expr_hdl(char *s);

int main(int argc, char *argv[]) {
  static char line[4096];
  while (1) {
    line_num ++;
    printf("crepl> ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    
    if (strstr(line, "int") == line) 
      func_hdl(line);
    else 
      expr_hdl(line);
    
    // printf("Got %zu chars.\n", strlen(line)); // ??
  }
}

void func_hdl(char *s) {
  char func_name[SZ_BUF];
  sprintf(func_name, "func_%d", line_num);

  char c_src[SZ_BUF];
  sprintf(c_src, "%s", s);

  void *handle = load(func_name, c_src);
  
  if (handle == NULL) {
    perror("invalid function");
    return ;
  }

  printf("OK\n");
}

void expr_hdl(char *s) {
  char func_name[SZ_BUF];
  sprintf(func_name, "expr_%d", line_num);

  char c_src[2 * SZ_BUF];
  sprintf(c_src, "int %s() { return %s; }", func_name, s);  

  void *handle = load(func_name, c_src);

  if (handle == NULL) {
    perror("invalid expression");
    return ;
  }

  int (* expr)() = dlsym(handle, func_name);
  assert(expr != NULL);

  printf("= %d\n", expr());
}

void* load(char *func_name, char *c_src) {
  char file_path[SZ_BUF];
  sprintf(file_path, "/tmp/%s_XXXXXX.c", func_name);

  char so_path[SZ_BUF + 3];
  sprintf(so_path, "%s.so", file_path);

  int fd = mkstemps(file_path, strlen(".c"));
  assert(fd != -1);

  write(fd, c_src, strlen(c_src));

  // gcc
  pid_t pid = fork();
  assert(pid != -1);

  if (pid == 0) {
    // child 
#if UINTPTR_MAX == 0xffffffff
    execlp("gcc", "gcc", "-fPIC", "-shared", "-w", "-m32", file_path, "-o", so_path, NULL); // Note: arg[0] must be the name of the bin
#else
    execlp("gcc", "gcc", "-fPIC", "-shared", "-w", "-m64", file_path, "-o", so_path, NULL); // Note: arg[0] must be the name of the bin
#endif

    exit(EXIT_FAILURE);
  } else {
    // parent
    wait(NULL);
    return dlopen(so_path, RTLD_NOW | RTLD_GLOBAL);
  }
  return NULL;
}
