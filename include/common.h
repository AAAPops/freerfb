//
// Created by urv on 10/9/21.
//

#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>
#include <stdlib.h>

#include "log.h"
#include "inbuf.h"
#include "tcp_connection.h"
#include "utils.h"

#define INBUFF_INIT_SZ  1024 * 1024 // 1Mb

#define SHARED_FLAG 0

typedef struct {
    uint8_t *ptr;
    size_t inuse;
    uint8_t *curr;
    size_t  total_sz;
} inbuff_struct;


struct pixel_format {
    uint8_t bits_per_pixel;
    uint8_t depth;
    uint8_t big_endian_flag;
    uint8_t true_colour_flag;
    uint16_t red_max;
    uint16_t green_max;
    uint16_t blue_max;
    uint8_t red_shift;
    uint8_t green_shift;
    uint8_t blue_shift;
    uint8_t padding[3];
};

typedef struct {
    uint16_t  framebuffer_x;
    uint16_t  framebuffer_y;
    struct pixel_format pixelFormat;
    uint32_t  name_length;
    char     *name_str;
} srv_framebuff_struct;


#endif  //FREERFB_COMMON_H
