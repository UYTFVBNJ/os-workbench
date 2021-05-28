/*
Note: 
1.  if I don't close it, getline will not return
2.  machine is always right [wulian]

Question:
1.  Why I need 
  while (sizeof(int *) == 4 || waitpid(-1, &wstatus, WNOHANG) == 0) 
    to prevent exit early in 32 bit mode?
*/
#include <assert.h>
#include <fcntl.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

int pipefd[2];

#define SZ_LINEBF 512

// syscalls
#define NR_TIMER 1000
#define SZ_SCLEN 128

struct timer {
  char name[SZ_SCLEN];
  suseconds_t time;
} timers[NR_TIMER];
int timer_cnt = 0;
suseconds_t timer_tot;

int get_syscall_num(char* str) {
  for (int i = 0; i < timer_cnt; i++)
    if (strcmp(timers[i].name, str) == 0) {
      return i;
    }
  assert(timer_cnt < NR_TIMER);
  strcpy(timers[timer_cnt].name, str);
  timers[timer_cnt].time = 0;
  return timer_cnt++;
}

void show() {
  static int show_time = 0;
  printf("Time #%d\n", ++ show_time);

  // printf("Timer: %ld, %d\n", timer_tot, timer_cnt);
  // for (int i = 0; i < timer_cnt; i ++) printf("%s %ld\n", timers[i].name, timers[i].time);

  for (int t = 0; t < 5; t ++) {
    int maxp = -1;
    suseconds_t maxv = -1;
    for (int i = 0; i < timer_cnt; i ++) 
      if (timers[i].time > maxv) {
        maxp = i;
        maxv = timers[i].time;
      }

    if (maxp != -1) {
      printf("%s (%ld%%)\n", timers[maxp].name, timers[maxp].time * 100 / timer_tot);
      // printf("%s (%ld.%6lds)\n", timers[maxp].name, timers[maxp].time / 1000000, timers[maxp].time % 1000000);
      timers[maxp].time = -timers[maxp].time;
    }
  }

  for (int i = 0; i < 80; i ++) putchar('\0');

  fflush(stdout);

  for (int i = 0; i < timer_cnt; i ++) 
    if (timers[i].time < 0) timers[i].time = -timers[i].time;
}

void child(int argc, char* argv[], char* envp[]) {
  char* exec_argv[40] = {
    "strace",
    "-T",
    "-xx",
  };

  memcpy(exec_argv + 3, argv + 1, argc * (sizeof (char *)));
  assert(3 + argc + 1 < 40);

  // printf("%d %d\n", argc, (int)(argc * (sizeof (char *))));

  // for (int i = 0; i < 10; i ++) printf("%d %s\n", i, exec_argv[i]);
  // for (int i = 0; envp[i] != NULL; i ++) printf("%d %s\n", i, envp[i]);

  // int fd = open("./sperf.out", O_CREAT | O_WRONLY, S_IRUSR);
  int fd = open("/dev/null", 0);
  assert(fd != -1);

  dup2(fd, STDOUT_FILENO);
  dup2(fd, STDERR_FILENO);
  close(fd);

  dup2(pipefd[1], STDERR_FILENO);
  close(pipefd[1]);

  char PATH[512];
  
  strcpy(PATH, getenv("PATH"));

  for (char *path = strtok(PATH, ":"); path != NULL; path = strtok(NULL, ":")) {
    char pathname[256];
    strcpy(pathname, path);
    strcat(pathname, "/strace");

    if (access(pathname, X_OK) == 0) 
      execve(pathname, exec_argv, envp);
  }
}

void parent() {
  FILE* stat = fdopen(pipefd[0], "r");

  char *null = NULL;
  char** buf = &null;
  size_t n = 0;
  int wstatus = 0;

  struct timeval next_frame;
  gettimeofday(&next_frame, NULL);
  next_frame.tv_sec += 1;

  while (sizeof(int *) == 4 || waitpid(-1, &wstatus, WNOHANG) == 0) {
    getline(buf, &n, stat);
    // printf("len %d\n", (int)strlen(*buf));
    assert(*buf != NULL);
    // printf("wst: %d\n", wstatus);
    // printf("%s", *buf);
    // printf("%p\n", *buf);
    assert(*buf[0] != '\0');
    struct timeval current_time;
 
    if (!((*buf)[0] == '+' && (*buf)[1] == '+' && (*buf)[2] == '+' && 
        !((*buf)[0] == '-' && (*buf)[1] == '-' && (*buf)[2] == '-'))) {
      char sc_name[SZ_LINEBF];
      struct timeval time = {-1, -1};
   
      sscanf(*buf, "%[^(] %*[^<] <%ld.%ld>", sc_name, &time.tv_sec, &time.tv_usec); 
      
      // printf("%p\n", *buf);
      if (!(time.tv_sec == -1 && time.tv_usec == -1 && strstr(*buf, "= ?") != NULL)) {
        while (time.tv_sec == -1 && time.tv_usec == -1) {
          getline(buf, &n, stat);
          sscanf(*buf, "%*[^<] <%ld.%ld>", &time.tv_sec, &time.tv_usec); 
        }

        assert(time.tv_sec != -1 && time.tv_usec != -1);
        int num = get_syscall_num(sc_name);
        suseconds_t time_us = time.tv_sec * 1000000 + time.tv_usec;
        timers[num].time += time_us;
        timer_tot += time_us;
      }
    }

    gettimeofday(&current_time, NULL);
    if (current_time.tv_sec > next_frame.tv_sec || 
    (current_time.tv_sec == next_frame.tv_sec && current_time.tv_usec > next_frame.tv_usec)) {
      next_frame.tv_sec += 1;
      puts("show normally");
      show();
      // timer_clear();
    }
  }

  show();
  exit(0);
}

int main(int argc, char* argv[], char* envp[]) {
  assert(pipe(pipefd) == 0);

  pid_t pid = fork();
  assert(pid >= 0);

  if (pid == 0) {
    close(pipefd[0]);
    child(argc, argv, envp);
    assert(0);
  } else {
    close(pipefd[1]); // Note: if I don't close it, getline whill not return
    parent();
  }

  perror(argv[0]);

  exit(EXIT_FAILURE);
}
