/*
 * Todo: Put description here
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "tcp_connection.h"
#include "log.h"

char *msg[] = {"Hello", "How are you", "Bye",
               "Long Long string of text 123456789987456321097854612302125467893215648",
               NULL};

#define SRV_ADDR_STR    "127.0.0.1:7777"
#define IP_ADDR_SZ      sizeof("127.127.127.127") + 1
#define IP_PORT_SZ      sizeof("65000") + 1


int main(int argc, char* argv[])
{
    char ipaddr[IP_ADDR_SZ] = {0};
    char ipport[IP_PORT_SZ] = {0};

    int client_fd;
    int retcode = -1;
    int ret;

    log_set_level(LOG_INFO);
    log_set_quiet(0);


    strToAddrPort(SRV_ADDR_STR, ipaddr, ipport);

    client_fd = conn_to_srv(ipaddr, ipport);
    if( client_fd < 0 ) {
        log_fatal("Not connection to server %s:%s", ipaddr, ipport);
        goto err;
    }

    int idx;
    for( idx = 0; ;idx++ ) {
        if( msg[idx] == NULL)
            break;

        log_debug("Send msg '%s'", msg[idx]);
        ret = send_payload(client_fd, (uint8_t*)msg[idx], strlen(msg[idx]));
        if( ret != 0 ) {
            log_fatal("Not able to send msg '%s' to server", msg[idx]);
        }
        usleep(10000);
    }

    retcode = 0;
    goto out;

err:
    retcode = -1;
out:
    if( client_fd > 0 )
        close_fd(client_fd, "Client fd close");

    return retcode;
}

