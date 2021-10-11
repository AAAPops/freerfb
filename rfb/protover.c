#include "../common/common.h"
#include "../log/log.h"
#include "../common/inbuf.h"
#include "../connection/tcp_connection.h"
#include "protover.h"

extern inbuff_struct inBuff;

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