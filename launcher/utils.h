#ifndef _UTILS_H
#define _UTILS_H

float format_size(float len);
const char *format_size_str(uint64_t len);

void copy_file(const char *src, const char *dst);
void recursive_rmdir(const char *path);
void recursive_mkdir(char *dir);

#endif
