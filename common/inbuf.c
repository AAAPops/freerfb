#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "common.h"
#include "inbuf.h"

extern inbuff_struct inBuff;

int inBuff_init(size_t buff_size) {
    inBuff.ptr = (uint8_t*)malloc(buff_size);
    if( !inBuff.ptr ) {
        log_fatal("Not able to create Buffer for input messages");
        return -1;
    }

    inBuff.inuse = 0;
    inBuff.curr = inBuff.ptr;
    inBuff.total_sz = INBUFF_INIT_SZ;

    return 0;
}

void inBuff_reset(void) {
    inBuff.inuse = 0;
    inBuff.curr = inBuff.ptr;
}

void inBuff_expand(void) {

}