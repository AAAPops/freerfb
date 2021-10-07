#ifndef _CONNECTION_H
#define _CONNECTION_H   1

#include <stdio.h>
#include <stdint.h>
#include <getopt.h>

#define POLL_TIMEOUT  5 * 1000
#define MEMZERO(x)	memset(&(x), 0, sizeof (x));


/******************* Server's functions *******************/
int srv_init(const char *addr, const char *port);
int srv_accept_conn(int srv_fd, const char *addr_str, const char *port);
int srv_unix_sock_init (char *socket_path);

/******************* Send/Receive functions *******************/
int send_payload(int fd, uint8_t *buff, size_t buff_sz);
ssize_t recv_payload(int fd, uint8_t *buff, size_t buff_sz, ssize_t total_read);

/********************* Client's functions *********************/
int conn_to_srv(const char *addr, const char *port);
int conn_to_unix_socket(const char *socket_path);

/********************** Misc Functions ************************/
void close_fd(int fd, const char *str);
int strToAddrPort(const char *argument, char *ipaddr, char *ipport);



#endif  // End _CONNECTION_H