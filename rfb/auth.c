#include <arpa/inet.h>

#include "../common/common.h"
#include "../log/log.h"
#include "../common/inbuf.h"
#include "../connection/tcp_connection.h"
#include "../utils/utils.h"
#include "auth.h"

extern inbuff_struct inBuff;

int security_phase(int fd)
{
    log_set_level(LOG_TRACE);
    log_trace("%s()", __FUNCTION__);

    uint8_t aSecType[SEC_TYPES_MAX] = {0};

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
        log_trace("aSecType[%d] = %d", arr_idx, tmp_val);
    }

    if( aSecType[2] != VNC_AUTH )
        return -1;

    uint8_t security_type = VNC_AUTH;

    send_payload(fd, &security_type, 1);
    log_debug(">>> Will use VNC auth: #%d", security_type);


    inBuff_reset();
    nbytes = recv_payload(fd, inBuff.ptr, inBuff.total_sz, CHALLENGE_SZ);
    log_trace("nbytes = %d", nbytes);
    if (nbytes < 0) {
        log_fatal("Server [fd = #%d] closed connection", fd);
        return -1;
    }
    inBuff.inuse = nbytes;

    if( log_get_level() == LOG_TRACE )
        memdump("<<< Challange", inBuff.curr, inBuff.inuse, 255);

    char *str1 =  memdump2str(inBuff.curr, inBuff.inuse);
    log_debug("<<< Challange: '%s'", str1);
    free(str1);


    uint8_t challenge_response[] = {0x68, 0xcb, 0x40, 0xf9, 0x82, 0xeb, 0x7a,
                                    0x91, 0xd1, 0x18, 0xaf, 0x39, 0x30, 0x02, 0x72, 0x41};

    send_payload(fd, challenge_response, sizeof(challenge_response));
    char *str2 =  memdump2str(challenge_response, sizeof(challenge_response));
    log_debug(">>> Challange response: '%s'", str2);
    free(str2);

    inBuff_reset();
    nbytes = recv_payload(fd, inBuff.ptr, inBuff.total_sz, READ_ALL_DATA);
    log_trace("nbytes = %d", nbytes);
    if (nbytes < 0) {
        log_fatal("Server [fd = #%d] closed connection", fd);
        return -1;
    }
    inBuff.inuse = nbytes;

    if( log_get_level() == LOG_TRACE )
        memdump("<<< SecurityResult", inBuff.curr, inBuff.inuse, 64);

    uint32_t sec_result = htonl(*(uint32_t*)inBuff.curr);
    if( sec_result != 0 ) {
        log_fatal("Authentication: Authentication failed");
        return -1;
    }

    return 0;
}