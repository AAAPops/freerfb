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
#include "common/inbuf.h"
#include "rfb/protover.h"
#include "rfb/auth.h"


#define ARGV_LAST_STR   "127.0.0.1:5901"
#define LOG_DEF_LEVEL   LOG_DEBUG
//#define LOG_DEF_LEVEL   LOG_TRACE

#define IP_ADDR_ARR_SZ  sizeof("127.127.127.127") + 1
#define IP_PORT_ARR_SZ  sizeof("65000") + 1


enum {PEERFD = 0, KBDFD, MAXFD, MOUSEFD, DISPFD};

enum {PROTOVER_PH, SEC_PH, INIT_PH, MSG_PH, ERR_PH = -1} currPhase;

inbuff_struct inBuff;


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
