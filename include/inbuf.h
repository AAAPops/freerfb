#ifndef _INBUFF_H
#define _INBUFF_H

int inBuff_init(size_t buff_size);

void inBuff_reset(void);

void inBuff_expand(void);

#endif