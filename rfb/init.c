//
// Created by urv on 10/15/21.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "common.h"

extern inbuff_struct inBuff;
extern srv_framebuff_struct srvFramebuff;

int init_phase(int fd) {
    log_debug("%s()", __FUNCTION__ );
    inBuff_reset();


    ssize_t nbytes = recv_payload(fd, inBuff.ptr, inBuff.total_sz, READ_ALL_DATA);
    log_trace("nbytes = %d", nbytes);
    if (nbytes < 0) {
        log_fatal("Server [fd = #%d] closed connection", fd);
        return -1;
    }
    inBuff.inuse = nbytes;

    memdump("", inBuff.ptr, inBuff.inuse, 32);

    log_debug("<<< Server framebuffer parameters [len=%d]", inBuff.inuse);

    srvFramebuff.framebuffer_x = NTOHS(inBuff.curr + 0);
    srvFramebuff.framebuffer_y = NTOHS(inBuff.curr + 2);
    srvFramebuff.name_length = NTOHL(inBuff.curr + 20);

    srvFramebuff.name_str = (char*)calloc(srvFramebuff.name_length + 3, sizeof(char));
    memcpy(srvFramebuff.name_str, inBuff.curr + 24, srvFramebuff.name_length);

    log_debug("\tSrv Framebuffer X = %d", srvFramebuff.framebuffer_x);
    log_debug("\tSrv Framebuffer Y = %d", srvFramebuff.framebuffer_y);
    log_debug("\tSrv Name length = %d", srvFramebuff.name_length);
    log_debug("\tSrv Name: '%s'", srvFramebuff.name_str);

    inBuff.curr += 4;
    srvFramebuff.pixelFormat.bits_per_pixel = NTOHC(inBuff.curr + 0);
    srvFramebuff.pixelFormat.depth = NTOHC(inBuff.curr + 1);
    srvFramebuff.pixelFormat.big_endian_flag = NTOHC(inBuff.curr + 2);
    srvFramebuff.pixelFormat.true_colour_flag = NTOHC(inBuff.curr + 3);
    srvFramebuff.pixelFormat.red_max = NTOHS(inBuff.curr + 4);
    srvFramebuff.pixelFormat.green_max = NTOHS(inBuff.curr + 6);
    srvFramebuff.pixelFormat.blue_max = NTOHS(inBuff.curr + 8);
    srvFramebuff.pixelFormat.red_shift = NTOHC(inBuff.curr + 10);
    srvFramebuff.pixelFormat.green_shift = NTOHC(inBuff.curr + 11);
    srvFramebuff.pixelFormat.blue_shift = NTOHC(inBuff.curr + 12);

    log_debug("\tSrv bits-per-pixel = %d", srvFramebuff.pixelFormat.bits_per_pixel);
    log_debug("\tSrv depth = %d", srvFramebuff.pixelFormat.depth);
    log_debug("\tSrv big-endian-flag = %d", srvFramebuff.pixelFormat.big_endian_flag);
    log_debug("\tSrv true-colour-flag = %d", srvFramebuff.pixelFormat.true_colour_flag);
    log_debug("\tSrv red-max = %d", srvFramebuff.pixelFormat.red_max);
    log_debug("\tSrv green-max = %d", srvFramebuff.pixelFormat.green_max);
    log_debug("\tSrv blue_max = %d", srvFramebuff.pixelFormat.blue_max);
    log_debug("\tSrv red-shift = %d", srvFramebuff.pixelFormat.red_shift);
    log_debug("\tSrv green-shift = %d", srvFramebuff.pixelFormat.green_shift);
    log_debug("\tSrv blue-shift = %d", srvFramebuff.pixelFormat.blue_shift);


/*
    uint8_t shared_flag = SHARED_FLAG;
    send_payload(fd, &shared_flag, 1);
    log_debug(">>> Share VNC desktop: %s", (shared_flag = 1) ? "Yes" : "No");
*/

    return 0;
}
