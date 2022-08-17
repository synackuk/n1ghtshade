.text

.globl _exit
.globl _fork
.globl _read
.globl _write
.globl _open
.globl _close
.globl _unlink
.globl _chdir

.globl _mlock
.globl _mkdir
.globl _rmdir
.globl _unmount

/*
.global _mknod: 0xD  int mknod(const char *pathname, mode_t mode, dev_t dev)                      3
*/
.globl _chmod
.globl _chown
.globl _sync
.globl _kill
.globl _dup
.globl _symlink

/*
.global _execve: 0x3B  int execve(const char *filename, char *const argv[], char *const envp[])   3
*/

.globl _chroot
.globl _vfork
.globl _reboot

/*
.global _munmap: 0x49  int munmap(void *addr, size_t length)                                 2
.global _mprotect: 0x4A  int mprotect(const void *addr, size_t len, int prot)                     3
*/

.globl _dup2
.globl _mount
.globl _stat
.globl _pread
.globl _pwrite

/*
.global _mmap: 0xC5  void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) 6
*/

.globl _access
.globl _wait4
.globl _execve

.align 4

/* void exit(int status) */
_exit:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x4
    str    r0, [sp,#0x0]
    mov    r12, #0x1
    swi    #0x80
    add    sp, sp, #0x4
    ldmfd  sp!, {r12,pc}

.align 4

/* pid_t fork(void) */
_fork:
    stmfd  sp!, {r12,lr}
    mov    r12, #0x2
    swi    #0x80
    ldmfd  sp!, {r12,pc}
    
.align 4

/* ssize_t read(int fd, void *buf, size_t count) */
_read:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0xC
    str    r0, [sp,#0x8]
    str    r1, [sp,#0x4]
    str    r2, [sp,#0x0]
    mov    r12, #0x3
    swi    #0x80
    add    sp, sp, #0xC
    ldmfd  sp!, {r12,pc}

.align 4
 
/* ssize_t write(int fd, const void *buf, size_t count) */
_write:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0xC
    str    r0, [sp,#0x8]
    str    r1, [sp,#0x4]
    str    r2, [sp,#0x0]
    mov    r12, #0x4
    swi    #0x80
    add    sp, sp, #0xC
    ldmfd  sp!, {r12,pc}
    

.align 4


/* int open(const char *pathname, int flags, mode_t mode) */
_open:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0xC
    str    r0, [sp,#0x8]
    str    r1, [sp,#0x4]
    str    r2, [sp,#0x0]
    mov    r12, #0x5
    swi    #0x80
    add    sp, sp, #0xC
    ldmfd  sp!, {r12,pc}

.align 4


/* int close(int fd) */
_close:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x4
    str    r0, [sp,#0x0]
    mov    r12, #0x6
    swi    #0x80
    add    sp, sp, #0x4
    ldmfd  sp!, {r12,pc}
  
.align 4


/* int unlink(const char *pathname); */
_unlink:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x4
    str    r0, [sp,#0x0]
    mov    r12, #0xA
    swi    #0x80
    add    sp, sp, #0x4
    ldmfd  sp!, {r12,pc}

.align 4


/* int chdir(const char *path) */
_chdir:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x4
    str    r0, [sp,#0x0]
    mov    r12, #0xC
    swi    #0x80
    add    sp, sp, #0x4
    ldmfd  sp!, {r12,pc}

.align 4


/* int chmod(const char *path, int mode) */
_chmod:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x8
    str    r0, [sp,#0x4]
    str    r1, [sp,#0x0]
    mov    r12, #0xF
    swi    #0x80
    add    sp, sp, #0x8
    ldmfd  sp!, {r12,pc}

.align 4


/* int chown(const char *path, int owner, int group) */
_chown:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x10
    str    r0, [sp,#0xC]
    str    r1, [sp,#0x8]
    str    r2, [sp,#0x4]
    str    r3, [sp,#0x0]
    mov    r12, #0x10
    swi    #0x80
    add    sp, sp, #0x10
    ldmfd  sp!, {r12,pc}

.align 4


/* void sync(void) */
_sync:
    stmfd  sp!, {r12,lr}
    mov    r12, #0x24
    swi    #0x80
    ldmfd  sp!, {r12,pc}

.align 4


/* int mlock(const void *addr, size_t len) */
_mlock:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x8
    str    r0, [sp,#0x4]
    str    r1, [sp,#0x0]
    mov    r12, #0xCB
    swi    #0x80
    add    sp, sp, #0x8
    ldmfd  sp!, {r12,pc}

.align 4


/* int mkdir(const char *path, mode_t mode) */
_mkdir:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x8
    str    r0, [sp,#0x4]
    str    r1, [sp,#0x0]
    mov    r12, #0x88
    swi    #0x80
    add    sp, sp, #0x8
    ldmfd  sp!, {r12,pc}

.align 4


/* int rmdir(const char *path) */
_rmdir:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x4
    str    r0, [sp,#0x0]
    mov    r12, #0x89
    swi    #0x80
    add    sp, sp, #0x4
    ldmfd  sp!, {r12,pc}

.align 4


/* int unmount(const char *dir, int flags) */
_unmount:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x8
    str    r0, [sp,#0x4]
    str    r1, [sp,#0x0]
    mov    r12, #0x9F
    swi    #0x80
    add    sp, sp, #0x8
    ldmfd  sp!, {r12,pc}

.align 4


/* int dup2(int oldfd, int newfd) */
_dup2:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x8
    str    r0, [sp,#0x4]
    str    r1, [sp,#0x0]
    mov    r12, #0x5A
    swi    #0x80
    add    sp, sp, #0x8
    ldmfd  sp!, {r12,pc}

.align 4


/* int stat(const char *path, struct stat *buf) */
_stat:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x8
    str    r0, [sp,#0x4]
    str    r1, [sp,#0x0]
    mov    r12, #0xBC
    swi    #0x80
    add    sp, sp, #0x8
    ldmfd  sp!, {r12,pc}

.align 4


/* int mount(const char *filesystemtype, const char *target, int mountflags, void *source) */
_mount:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x10
    str    r0, [sp,#0xC]
    str    r1, [sp,#0x8]
    str    r2, [sp,#0x4]
    str    r3, [sp,#0x0]
    mov    r12, #0xA7
    swi    #0x80
    add    sp, sp, #0x10
    ldmfd  sp!, {r12,pc}

.align 4


/* ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset) */
_pread:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x10
    str    r0, [sp,#0xC]
    str    r1, [sp,#0x8]
    str    r2, [sp,#0x4]
    str    r3, [sp,#0x0]
    mov    r12, #0x99
    swi    #0x80
    add    sp, sp, #0x10
    ldmfd  sp!, {r12,pc}

.align 4

/* ssize_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset) */
_pwrite:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x10
    str    r0, [sp,#0xC]
    str    r1, [sp,#0x8]
    str    r2, [sp,#0x4]
    str    r3, [sp,#0x0]
    mov    r12, #0x9A
    swi    #0x80
    add    sp, sp, #0x10
    ldmfd  sp!, {r12,pc}

.align 4


/* int kill(pid_t pid, int sig) */
_kill:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x8
    str    r0, [sp,#0x4]
    str    r1, [sp,#0x0]
    mov    r12, #0x25
    swi    #0x80
    add    sp, sp, #0x8
    ldmfd  sp!, {r12,pc}

.align 4


/* int dup(int oldfd) */
_dup:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x4
    str    r0, [sp,#0x0]
    mov    r12, #0x29
    swi    #0x80
    add    sp, sp, #0x4
    ldmfd  sp!, {r12,pc}

.align 4


/* int symlink(const char *oldpath, const char *newpath) */
_symlink:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x8
    str    r0, [sp,#0x4]
    str    r1, [sp,#0x0]
    mov    r12, #0x39
    swi    #0x80
    add    sp, sp, #0x8
    ldmfd  sp!, {r12,pc}

.align 4


/* int chroot(const char *path) */
_chroot:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x4
    str    r0, [sp,#0x0]
    mov    r12, #0x3D
    swi    #0x80
    add    sp, sp, #0x4
    ldmfd  sp!, {r12,pc}

.align 4


/* pid_t vfork(void) */
_vfork:
  eor r0, r0, r0
  mov r12, #0x42
  swi 128
  cmp r1, #0x0
  beq vfork_parent
  mov r0, #0x0
.align 4
vfork_parent:
  bx lr

.align 4


/* int reboot(int howto) */
_reboot:
    stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x4
    str    r0, [sp,#0x0]
    mov    r12, #0x37
    swi    #0x80
    add    sp, sp, #0x4
    ldmfd  sp!, {r12,pc}

.align 4


/* access(const char *path, int amode) */
_access:
	stmfd  sp!, {r12,lr}
    sub    sp, sp, #0x8
    str    r0, [sp,#0x4]
    str    r1, [sp,#0x0]
    mov    r12, #0x21
    swi    #0x80
    add    sp, sp, #0x8
    ldmfd  sp!, {r12,pc}	

.align 4


/* pid_t wait4(pid_t pid, int *stat_loc, int options, struct rusage *rusage) */
_wait4:
    mov    r12, #0x7
    swi    #0x80
    bx     lr

.align 4


/* int execve(const char *path, char *const argv[], char *const envp[]) */
_execve:
    mov    r12, #0x3B
    swi    #0x80
    bx     lr