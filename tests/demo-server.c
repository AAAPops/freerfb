/*
 * Todo: Put description here
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <poll.h>
#include <string.h>

#include "../connection/tcp_connection.h"
#include "../log/log.h"

#define SRV_STR     "127.0.0.1:7777"
#define IP_ADDR_SZ  sizeof("127.127.127.127") + 1
#define IP_PORT_SZ  sizeof("65000") + 1

#define BUFF_SZ     1024

#define MEMZERO(x)	memset(&(x), 0, sizeof (x));

int main(int argc, char* argv[])
{
    char ipaddr[IP_ADDR_SZ] = {0};
    char ipport[IP_PORT_SZ] = {0};

    int client_fd = -1;

    int pollResult;
    struct pollfd pollfds[2];
    int fds_count = 0;

    //int idx;
    ssize_t nbytes = 0;
    uint8_t buff[BUFF_SZ] = {0};



    strToAddrPort(SRV_STR, ipaddr, ipport);
    printf("ipaddr: %s,  ipport: %s \n", ipaddr, ipport);
    int srv_fd = srv_init(ipaddr, ipport);

    /*********************************/
    /*           Main loop           */
    /*********************************/
    while (1) {
        MEMZERO(pollfds);
        pollfds[0].fd = srv_fd;
        pollfds[0].events = POLLIN;
        fds_count = 1;

        if( client_fd > 0 ) {
            pollfds[1].fd = client_fd;
            pollfds[1].events = POLLIN;
            fds_count += 1;
        } else {
            pollfds[1].fd = -1;
        }
        log_trace("fds_count = %d", fds_count);

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


        // ----------- New client accept -----------
        if (pollfds[0].revents & POLLIN) {
            pollfds[0].revents = 0;

            int newsockfd = srv_accept_conn(srv_fd, ipaddr, ipport);
            if (newsockfd <= 0) {
                log_warn("accept(): {%m]");
            } else {
                log_info("Accept new client (fd = %d)", newsockfd);

                if( client_fd > 0) {
                    log_debug("Too much clients. Skip this one");
                } else {
                    log_debug("Client [fd = #%d] is free. Work with it", newsockfd);
                    client_fd = newsockfd;
                }
            }
        }



        // ----------- Get data from client and and send it back -----------
        if (pollfds[1].revents & POLLIN) {
            log_debug("Client [fd = #%d] has data. Read it", client_fd);
            pollfds[1].revents = 0;

            nbytes = recv_payload(client_fd, buff, sizeof(buff), -1);
            log_debug("nbytes = %d", nbytes);
            if( nbytes < 0 ) {
                log_warn("Client [fd = #%d] closed connection", client_fd);
                close_fd(client_fd, "Client closed connection");
                client_fd = -1;
            } else {
                char *tmp_str = (char*)malloc(nbytes);
                memset(tmp_str, 0, nbytes);
                strncpy(tmp_str, (char*)buff, nbytes - 2);
                printf("Get msg from client: %s \n", tmp_str);
                free(tmp_str);
                //return 0;
            }
        }

    } // End of While(1)



out:
    if( client_fd > 0 )
        close_fd(client_fd, "Client fd close");

    if( srv_fd > 0 )
        close_fd(srv_fd, "Server fd close");

    return 0;
}
