#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
//#include <stdio.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
//  printf("%p\n", args);
  int argc = *(int *)(args);
//  argc = *argc_addr;
  assert((uintptr_t)args == 0x7ffffffc);
  char ** argv = (char **)(args - 1 - argc);
  char *empty[] = {NULL };
  environ = empty;
//  printf("main addr:%p\n", (void *)main);
  exit(main(argc, argv, empty));
  assert(0);
}
