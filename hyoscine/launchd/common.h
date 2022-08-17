#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <syscalls.h>
#include <hfs_mount.h>

#define puts _puts

extern char* cache_env[];

int cp(const char *src, const char *dest);
int install(const char* src, const char* dst, int uid, int gid, int mode);
void sleep(unsigned int seconds);
int fsexec(char* argv[], char* env[]);
int exec(char* argv[], char* env[]);
void _puts(const char* s);
int hfs_mount(char* device, const char* mountdir, int options);

#endif