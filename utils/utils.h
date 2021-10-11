//
// Created by urv on 5/19/21.
//

#ifndef _UTILS_H
#define _UTILS_H

#include <string.h>

#define MEMZERO(x)	memset(&(x), 0, sizeof (x));

double stopwatch(char* label, double timebegin);

void memdump(char *name, uint8_t *buff, size_t len, uint8_t column_n);

char * memdump2str(uint8_t *buff, uint8_t len);

#endif // _UTILS_H
