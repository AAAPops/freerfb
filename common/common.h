//
// Created by urv on 10/9/21.
//

#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>
#include <stdlib.h>


#define INBUFF_INIT_SZ  1024 * 1024 // 1Mb

typedef struct {
    uint8_t *ptr;
    size_t inuse;
    uint8_t *curr;
    size_t  total_sz;
} inbuff_struct;



#endif  //FREERFB_COMMON_H
