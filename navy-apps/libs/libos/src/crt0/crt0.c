#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
//#include <stdio.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
//  printf("%p\n", args);
  assert(args != NULL);
  assert(main != NULL);
  int argc = *(int *)(args);
 // assert(0);
  //  argc = *argc_addr;
//  assert((uintptr_t)args == 0x7ffffffc);
  char ** argv = (char **)(args + 1);
  char ** envp = (char **)(args + 2 + argc);
  environ = envp;
//  printf("main addr:%p\n", (void *)main);
  exit(main(argc, argv, envp));
  assert(0);
}
