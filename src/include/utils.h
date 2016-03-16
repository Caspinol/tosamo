#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

char *to_time_now(void);
char *to_str_reverse(char const *, size_t);
void to_str_del_substr(char *, const char *, size_t);
char *to_itos(int, char[]);
size_t to_get_filelen(FILE *);
void to_str_trim(char *);
int to_is_big_endiann(void);

#endif
