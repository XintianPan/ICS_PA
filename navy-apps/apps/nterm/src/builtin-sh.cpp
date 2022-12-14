#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>
#include <stdio.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static int builtin_echo(char *arg){
	if(arg == NULL) sh_printf("\n");
	else sh_printf("%s", arg);
	return 0;
}

static int builtin_help(char *arg){
	sh_printf("help\necho\n");
	return 0;
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static struct{
	const char *name;
	int (*handler) (char *);
} builtin_cmd[] = {
	{"help", builtin_help},
	{"echo", builtin_echo},
};

#define CMD_LEN sizeof(builtin_cmd)/sizeof(builtin_cmd[0])

static void sh_handle_cmd(const char *cmd) {
	char buf[256];
	strcpy(buf, cmd);
	char *cmd_name = strtok(buf, " ");
	char *args;
	args = buf + strlen(cmd_name) + 1;
	if(args >= buf + strlen(cmd)) args = NULL, cmd_name = strtok(cmd_name, "\n");
	for(int i = 0; i < CMD_LEN; ++i){
		printf("%s %s\n", cmd_name, builtin_cmd[i].name);
		if(strcmp(builtin_cmd[i].name, cmd_name) == 0){ builtin_cmd[i].handler(args); return;}
	}
	strcpy(buf, cmd);
	int l = strlen(cmd);
	buf[l - 1] = '\0';
	char * file_name = strtok(buf, " ");
	if(execvp(file_name, NULL) == -1) sh_printf("file %s does not exists!\n", file_name);
}
void builtin_sh_run() {
  sh_banner();
  sh_prompt();
  setenv("PATH","/bin", 1);
  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
