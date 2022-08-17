#ifndef JUMPTO_H
#define JUMPTO_H

typedef void(*entry_t)(void* arg0, int, int, int);

__attribute__((noreturn)) void* jumpto(entry_t entry, void* args);

#endif