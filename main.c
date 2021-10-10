/*
 * Todo: RFB client prototype here -)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <poll.h>
#include <string.h>

#include "connection/tcp_connection.h"
#include "log/log.h"
#include "utils/utils.h"
#include "common/common.h"


#define ARGV_LAST_STR   "127.0.0.1:5901"
#define LOG_DEF_LEVEL   LOG_DEBUG
//#define LOG_DEF_LEVEL   LOG_TRACE

#define IP_ADDR_ARR_SZ  sizeof("127.127.127.127") + 1
#define IP_PORT_ARR_SZ  sizeof("65000") + 1


enum {PEERFD = 0, KBDFD, MAXFD, MOUSEFD, DISPFD};





int fill_pollfds(struct pollfd *pollfds, int peerfd, int kbdfd)
{
    memset(pollfds, 0, MAXFD * sizeof(struct pollfd));

    pollfds[PEERFD].fd = peerfd;
    pollfds[PEERFD].events = POLLIN;

    if( kbdfd > 0 ) {
        pollfds[KBDFD].fd = kbdfd;
        pollfds[KBDFD].events = POLLIN;
    } else
        pollfds[KBDFD].fd = -1;

    return 0;
}

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


int protover_phase(int fd)
{
#define PROTO_VER_SZ 12

    log_trace("%s()", __FUNCTION__);
    inBuff_reset();

    ssize_t nbytes = recv_payload(fd, inBuff.ptr, inBuff.total_sz, READ_ALL_DATA);
    log_trace("nbytes = %d", nbytes);
    if (nbytes < 0) {
        log_fatal("Server [fd = #%d] closed connection", fd);
        return -1;
    }
    inBuff.inuse = nbytes;

    if( inBuff.inuse == PROTO_VER_SZ ) {
        log_debug("<<< %s", inBuff.curr);

        send_payload(fd, inBuff.ptr, inBuff.inuse);
        log_debug(">>> %s", inBuff.curr);
    } else {
        // Print Reason-string here
        log_fatal("<<< %s", inBuff.curr);
        return -1;
    }

    return 0;
}


int security_phase(int fd)
{
#define REASON_STR_OFFSET 5
#define SEC_TYPES_MAX 255

#define VNC_AUTH 2

    uint8_t aSecType[SEC_TYPES_MAX] = {0};

    log_trace("%s()", __FUNCTION__);
    inBuff_reset();


    ssize_t nbytes = recv_payload(fd, inBuff.ptr, inBuff.total_sz, READ_ALL_DATA);
    log_trace("nbytes = %d", nbytes);
    if (nbytes < 0) {
        log_fatal("Server [fd = #%d] closed connection", fd);
        return -1;
    }
    inBuff.inuse = nbytes;

    uint8_t num_of_sec_types = *(uint8_t*)inBuff.curr;

    log_debug("<<< number-of-security-types = %d", num_of_sec_types);
    if (num_of_sec_types == 0) {
        inBuff.curr += REASON_STR_OFFSET;
        log_fatal("<<< %s ", inBuff.curr);

        return -1;
    }

    inBuff.curr += 1;
    for(int i = 0; i < num_of_sec_types; i++) {
        inBuff.curr += i;

        uint8_t arr_idx = *(uint8_t*)(inBuff.curr);
        uint8_t tmp_val = *(uint8_t*)(inBuff.curr);
        aSecType[arr_idx] = tmp_val;
        log_debug("aSecType[%d] = %d", arr_idx, tmp_val);
    }

    if( aSecType[2] != VNC_AUTH )
        return -1;

    uint8_t security_type = VNC_AUTH;

    send_payload(fd, &security_type, 1);
    log_debug(">>> %d", security_type);
    

    return 0;
}



int main(int argc, char* argv[])
{
    int ret, retcode;
    int endlessloop = 1;

    char sAddr[IP_ADDR_ARR_SZ] = {0};
    char sPort[IP_PORT_ARR_SZ] = {0};

    int peerfd = -1;
    int kbdfd = -1;

    int pollResult;
    struct pollfd pollfds[MAXFD];
    int fds_count = MAXFD;

    //int idx;

    currPhase = PROTOVER_PH;

    log_set_level(LOG_DEF_LEVEL);


    ret = inBuff_init(INBUFF_INIT_SZ);

    ret = strToAddrPort(ARGV_LAST_STR, sAddr, sPort);
    if (ret != 0)
        goto err;
    log_trace("ip addr: %s,  ip port: %s", sAddr, sPort);

    peerfd = conn_to_srv(sAddr, sPort);
    log_trace("peerfd = %d", peerfd);
    if (peerfd < 0)
        goto err;



    /*********************************/
    /*           Main loop           */
    /*********************************/
    while( endlessloop ) {
        fill_pollfds(pollfds, peerfd, kbdfd);

        pollResult = poll(pollfds, fds_count, -1);
        log_trace("pollResult = %d", pollResult);

        // ----------- Error case -----------
        if (pollResult < 0) {
            log_warn("poll(%m)");
            continue;
        }

        // ----------- Timeout case -----------
        if (pollResult == 0) {
            log_trace("poll(timeout = 1)");
            continue;
        }

        // ----------- Get data from Server -----------
        if (pollfds[PEERFD].revents & POLLIN) {
            log_trace("Get msg from Server. Read it");
            pollfds[PEERFD].revents = 0;

            switch(currPhase) {
                case PROTOVER_PH:
                    ret = protover_phase(peerfd);
                    if( ret != 0 )
                        goto err;

                    currPhase = SEC_PH;
                    break;

                case SEC_PH:
                    ret = security_phase(peerfd);
                    if( ret != 0 )
                        goto err;

                    currPhase = INIT_PH;
                    break;

                default:
                    log_fatal("Unknown Message type...\n");
                    goto out;
            }
        }

    } // End of While(1)


err:
out:
    if( peerfd > 0 )
        close_fd(peerfd, "Client fd close");

    return 0;
}
