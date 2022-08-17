#ifndef CALLBACK_H
#define CALLBACK_H

int log_callback(const char* fmt, ...);
int progress_callback(double progress);
int error_callback(const char* fmt, ...);

#endif