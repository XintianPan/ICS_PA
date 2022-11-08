#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
} Finfo; 

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENTS, FD_DISPINFO, FD_FB};

size_t serial_write(const void *buf, size_t offset, size_t len);

size_t fb_write(const void *buf, size_t offset, size_t len);

size_t events_read(void *buf, size_t offset, size_t len);

size_t dispinfo_read(void *buf, size_t offset, size_t len); 

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
} 

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_EVENTS] = {"/dev/events", 0, 0, events_read, invalid_write},
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
  [FD_FB]     = {"/dev/fb", 0, 0, invalid_read, fb_write},
#include "files.h"
};

static size_t open_off[LENGTH(file_table)];

size_t ramdisk_read(void *buf, size_t offset, size_t len);

size_t ramdisk_write(const void *buf, size_t offset, size_t len); 

int fs_open(const char *pathname, int flags, int mode ){
	int i = 0;
	for(; i < LENGTH(file_table); ++i){ 
		if(strcmp(pathname, file_table[i].name) == 0){
			Log("Successfully open file: %s", pathname);
			break;
		}
	}
	assert(i < LENGTH(file_table));
	return i;
}

size_t fs_read(int fd, void *buf, size_t len){
	assert(fd < LENGTH(file_table));
//	printf("%d %d\n", open_off[fd], file_table[fd].size);	
	if(file_table[fd].read == NULL){
		if(open_off[fd] >= file_table[fd].size){
			Log("Cross the boundary of file!");
			return 0;
		}else if(open_off[fd] + len - 1 >= file_table[fd].size){
			len = file_table[fd].size + 1 - open_off[fd];
			size_t read_sl = ramdisk_read(buf, open_off[fd] + file_table[fd].disk_offset, len);
			open_off[fd] += read_sl;
			return read_sl;
	
		}else{
			size_t read_s = ramdisk_read(buf, open_off[fd] + file_table[fd].disk_offset, len);
			open_off[fd] += read_s;
			return read_s;
		} 
	}else{
		return file_table[fd].read(buf, 0, len);
	}
}

size_t fs_lseek(int fd, size_t offset, int whence){
	assert(fd < LENGTH(file_table));
	size_t ret;
	switch(whence){
		case SEEK_SET:
			open_off[fd] = offset, ret = offset;
			break;
		case SEEK_CUR:
			open_off[fd] += offset; ret = open_off[fd];
			break;
		case SEEK_END:
			open_off[fd] = file_table[fd].size + offset;
			ret = open_off[fd];
			break;
		default:
			panic("Should Not reach here");
	}
	return ret;
}

size_t fs_write(int fd, const void *buf, size_t len){ 
	assert(fd < LENGTH(file_table));
	if(file_table[fd].write == NULL){
		if(open_off[fd] + len - 1 >= file_table[fd].size){
			Log("cross the file boundary, reshape len");
			len = file_table[fd].size + 1 - open_off[fd];
		} 
		size_t ret = ramdisk_write(buf, open_off[fd] + file_table[fd].disk_offset, len);
		open_off[fd] += len;
		return ret;
	}else{
		return file_table[fd].write(buf, 0, len);
	}
}

int fs_close(int fd){
	assert(fd < LENGTH(file_table));
	open_off[fd] = 0;
	Log("Successfully close file: %s", file_table[fd].name);
	return 0;
}


void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size = 400 * 300 * sizeof(uint32_t);
}
