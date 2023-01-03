#include <proc.h>
#include "syscall.h"

/*
 * uintptr_t should treated as value, not pointer!
 * uintptr_t, intptr_t is same length as pointer
 * it can be casted to a pointer
 * but if not being casted, it's simply treated as value!!!
 * */
/*
static char *syscall_name[] = {
  "exit",
  "yield",
  "open",
  "read",
  "write",
  "kill",
  "getpid",
  "close",
  "lseek",
  "brk",
  "fstat",
  "time",
  "signal",
  "execve",
  "fork",
  "link",
  "unlink",
  "wait",
  "times",
  "gettimeofday"
};
*/
extern PCB *current;

bool file_check(const char *pathname);

void naive_uload(PCB *pcb, const char *filename); 

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);

void switch_boot_pcb();

int fs_open(const char *pathname, int flags, int mode);

int fs_close(int fd);

size_t fs_read(int fd, void *buf, size_t len);

size_t fs_write(int fd, const void *buf, size_t len);

size_t fs_lseek(int fd, size_t offset, int whence);

int sys_gettime(intptr_t tv, intptr_t tz){
	uint32_t *time_sets = (uint32_t *)tv;
	uint64_t usec = io_read(AM_TIMER_UPTIME).us;
	uint64_t sec = usec / 1000000;
	time_sets[0] = (uint32_t) sec;
	time_sets[1] = (uint32_t) usec;
	return 0;	
}

int mm_brk(PCB *pcb, uintptr_t brk);

void do_syscall(Context *c) {
  uintptr_t a[5];
  a[0] = c->GPR1; //a7
  a[1] = c->GPR2; //a0 used for argument
  a[2] = c->GPR3; //a1 used for argument
  a[3] = c->GPR4; //a2 used for argument
  a[4] = c->GPRx; //a0 used for return
   switch (a[0]) {
	case SYS_exit:
//	   	Log("syscall:%s 1st arg:%d 2nd arg:%d 3rd arg:%d", syscall_name[a[0]], a[1], a[2], a[3]);
//	   	Log("let's end this staff!");
//		if(a[1] != 0) assert(0);
//		halt(a[1]);
		context_uload(current, "/bin/nterm", NULL, NULL);
	   	switch_boot_pcb();
		yield();
		break;
	case SYS_yield: 
//		Log("syscall:%s 1st arg:%d 2nd arg:%d 3rd arg:%d", syscall_name[a[0]], a[1], a[2], a[3]);
		yield(); 
		c->mepc += 4; c->gpr[10] = 0; break;
	case SYS_open:
//		Log("syscall:%s 1st arg:%s 2nd arg:%d 3rd arg:%d", syscall_name[a[0]],(char *)a[1], a[2], a[3]);
		Log("open");
		c->gpr[10] = fs_open((char *)a[1], (int)a[2], (int)a[3]);
		c->mepc += 4;
		break;
	case SYS_read:
//		Log("syscall:%s 1st arg:%d 2nd arg:%p 3rd arg:%d", syscall_name[a[0]], a[1], a[2], a[3]);
		Log("read");
		c->gpr[10] = fs_read((int)a[1], (void *)a[2], a[3]);
		c->mepc += 4;
		break;
	case SYS_write: 
//	   	Log("syscall:%s 1st arg:%d 2nd arg:%p 3rd arg:%d", syscall_name[a[0]], a[1], a[2], a[3]); 
	    Log("write");
		size_t ret = fs_write(a[1], (void *)a[2], a[3]);
		c->gpr[10] = ret;
	   	c->mepc += 4;  
		break;
	case SYS_close:
//		Log("syscall:%s 1st arg:%d 2nd arg:%d 3rd arg:%d", syscall_name[a[0]], a[1], a[2], a[3]);
		Log("close");
		c->gpr[10] = fs_close(a[1]);
		c->mepc += 4;
		break;
	case SYS_lseek:
//		Log("syscall:%s 1st arg:%d 2nd arg:%d 3rd arg:%d", syscall_name[a[0]], a[1], a[2], a[3]);
		c->gpr[10] = fs_lseek(a[1], a[2], a[3]);
		c->mepc += 4;
		break;
	case SYS_brk:
//		Log("syscall:%s 1st arg:%d 2nd arg:%p 3rd arg:%d", syscall_name[a[0]], a[1], a[2], a[3]);
//		Log("%x", *(int*)a[2]);
		Log("brk");
		if(current->max_brk == 0) current->max_brk = *(uintptr_t *)a[2];
	    *(uintptr_t *)a[2] = *(uintptr_t *)a[2] + a[3];
//		Log("%x", *(int*)a[2]);
		c->mepc += 4;
		c->gpr[10] = mm_brk(current, *(uintptr_t *)a[2]);
		break;
	case SYS_gettimeofday:
//		Log("syscall:%s 1st arg:%p 2nd arg:%p 3rd arg:%d", syscall_name[a[0]], a[1], a[2], a[3]);
		c->gpr[10] = sys_gettime(a[1], a[2]);
		c->mepc += 4;
		break;
	case SYS_execve:
//		Log("%s", (char *)a[1]);
		if(file_check((char *)a[1])){
//			Log("%s", (char *)a[1]);
			context_uload(current, (char *)a[1], (char *const *)a[2], (char *const *)a[3]);
			switch_boot_pcb();
			yield();
			//	Log("here %x %x", c->gpr[2], c->mepc);
		}
		else
			c->gpr[10] = -2, c->mepc += 4;	
		break;
	default: panic("Unhandled syscall ID = %d", a[0]);
  } 
}
