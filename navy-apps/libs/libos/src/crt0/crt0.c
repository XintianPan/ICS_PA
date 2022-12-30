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
  char *empty[] = {NULL };
  environ = empty;
//  printf("main addr:%p\n", (void *)main);
  exit(main(argc, (char **)(args - 1 - argc), empty));
  assert(0);
}
