//
// Created by urv on 10/9/21.
//

#ifndef _COMMON_H
#define _COMMON_H

enum phase {PROTOVER_PH, SEC_PH, INIT_PH, MSG_PH, ERR_PH = -1} currPhase;

#define INBUFF_INIT_SZ  1024 * 1024 // 1Mb

struct in_buff {
    uint8_t *ptr;
    size_t inuse;
    uint8_t *curr;
    size_t  total_sz;
} inBuff;



#endif  //FREERFB_COMMON_H
