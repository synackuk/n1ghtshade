#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <syscalls.h>

#define puts _puts

extern char* cache_env[];

void sleep(unsigned int seconds);
void _puts(const char* s);
int fsexec(char* argv[], char* env[], int pause);
int hfs_mount(char* device, const char* mountdir, int options);
int install(const char* src, const char* dst, int uid, int gid, int mode);
int cp(const char *src, const char *dest);

#endif