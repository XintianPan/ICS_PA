#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  int argc;
  int* argc_addr = (int *)(args - sizeof(int));
  argc = *argc_addr;
  char **argv = (char **)(args - sizeof(int) - 1 - argc);
  uintptr_t *ev = (uintptr_t *)(args - sizeof(int) - 3 - argc);
  while(ev != NULL) --ev;
  char **envp = (char **)(ev - 1);
  environ = envp;
  exit(main(argc, argv, envp));
  assert(0);
}
