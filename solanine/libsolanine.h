#ifndef LIBSOLANINE_H
#define LIBSOLANINE_H

typedef void(*solanine_log_cb)(char* msg);

extern solanine_log_cb solanine_log;

#define LOG solanine_log

void libsolanine_set_log_cb(solanine_log_cb new_cb);
void libsolanine_init();
void libsolanine_exit();
int libsolanine_patch_ipsw(char* input, char* output, char* identifier);

#endif