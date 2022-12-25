#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
static int argc;
static char **argv;
static char **envp;
static int envpc;
void call_main(uintptr_t *args) {
  printf("%p\n", args);
  argc = *(int *)(args);
//  argc = *argc_addr;
  argv = (char **)(args - 1 - argc);
  envpc = *(int *)((int *)args - 2 - argc);
  envp = (char **)((int *)args - 3 - argc - envpc);
  environ = envp;
//  printf("main addr:%p\n", (void *)main);
  exit(main(argc, argv, envp));
  assert(0);
}
