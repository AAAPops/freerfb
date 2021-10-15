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

struct _tst1 {
    char *str;
    int result;
};

struct _tst1 tst1[] = {
        "127.0.0.1:7777",    0,
        "127.0.0.1:",       -1,
        ":5555",            -1,
        "127.0.0.303:7777", -1,
        "127.0.0.1:-7777",  -1,
        "127.0.0.1:77777",  -1,
        "128.0.0.:7777",    -1,
        "10.1.91.5:1340",    0,
        "128.0..0.1:7777",  -1,
        "128.0:7777",       -1
};


char *msg[] = {"Hello", "How are you", "Bye",
               "Long Long string of text 123456789987456321097854612302125467893215648",
               NULL};


#define TST1_SZ        sizeof(tst1)/sizeof(tst1[0])
#define CLNT_MSG_SZ    sizeof(clnt_msg)/sizeof(clnt_msg[0])

#define SRV_ADDR_STR     "127.0.0.1:7777"
#define IP_ADDR_SZ  sizeof("127.127.127.127") + 1
#define IP_PORT_SZ  sizeof("65000") + 1

#define BUFF_SZ     1024

#define MEMZERO(x)	memset(&(x), 0, sizeof (x));

int test_1(void) {
    int idx;
    int retcode;
    char ipaddr[IP_ADDR_SZ] = {0};
    char ipport[IP_PORT_SZ] = {0};

    for( idx = 0; idx < TST1_SZ ;idx++)
    {
        retcode = strToAddrPort(tst1[idx].str, ipaddr, ipport);
        if( retcode == tst1[idx].result ) {
            //printf("%s ---> ipaddr: %s,  ipport: %s \n", tst1[idx].str, ipaddr, ipport);
        } else {
            //printf("%s ---> Wrong input string \n", tst1[idx].str);
            printf("Test-1: Failed \n");
            return -1;
        }
    }
    printf("Test-1: Ok \n");

    return 0;
}

int main(int argc, char* argv[])
{
    char ipaddr[IP_ADDR_SZ] = {0};
    char ipport[IP_PORT_SZ] = {0};

    int client_fd;
    int retcode = -1;
    int ret;

    ssize_t nbytes = 0;
    uint8_t buff[BUFF_SZ] = {0};

    log_set_level(LOG_DEBUG);
    log_set_quiet(1);


    ret = test_1();
    if( ret != 0 )
        goto err;


    strToAddrPort(SRV_ADDR_STR, ipaddr, ipport);
    int srv_fd = srv_init(ipaddr, ipport);

    client_fd = srv_accept_conn(srv_fd, ipaddr, ipport);
    if (client_fd < 0) {
        log_warn("accept(): {%m]");
        goto err;
    } else {
        log_info("Accept new client (fd = %d)", client_fd);
    }



    int to_read;
    int idx;
    for( idx = 0; ;idx++ ) {
        if (msg[idx] == NULL)
            break;

        to_read = ( idx % 2 )?  -1 : (int)strlen(msg[idx]);
        log_debug("to_read[%d] = %d", idx, to_read);

        memset(buff, 0, BUFF_SZ);
        nbytes = recv_payload(client_fd, buff, BUFF_SZ, to_read);
        log_debug("Msg from client: '%s' [len=%d]", buff, nbytes);
        if (strncmp((char *) buff, msg[idx], strlen(msg[idx])) != 0)
            goto err;
    }


    printf("Test-2: Ok \n");
    retcode = 0;
    goto out;

err:
    printf("Test-2: Failed \n");
    retcode = -1;
out:
    if( client_fd > 0 )
        close_fd(client_fd, "Client fd close");

    if( srv_fd > 0 )
        close_fd(srv_fd, "Server fd close");

    return retcode;
}
