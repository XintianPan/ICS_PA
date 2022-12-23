#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  int argc = *(int *)(args);
//  argc = *argc_addr;
  char **argv = (char **)(args - 1 - argc);
  int envpc = *(int *)((int *)args - 2 - argc);
  char **envp = (char **)((int *)args - 3 - argc - envpc);
  environ = envp;
  exit(main(argc, argv, envp));
  assert(0);
}
