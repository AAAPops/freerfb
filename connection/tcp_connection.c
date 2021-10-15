#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include "common.h"
#include "tcp_connection.h"


/******************* Server's functions *******************/

/**
 * @param [in] addr string like "192.168.1.1"
 * @param [in] port string like "7777"
 * @param [out] srv_fd New Server's file descriptor
 * @return 0 if success
 * @return -1 if error
 */
int srv_init(const char *addr, const char *port)
{
    int srv_fd;
    struct sockaddr_in servaddr;
    //socklen_t peer_addr_size;
    uint16_t ip_port = (int)strtol(port, NULL, 10);

    struct in_addr inp;
    inet_aton(addr, &inp);

    // socket create and verification
    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if( srv_fd == -1 ) {
        log_fatal("Socket creation failed...[%m]");
        return -1;
    }

    int enable = 1;
    if( setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0 ) {
        log_fatal("setsockopt(SO_REUSEADDR) failed");
        return -1;
    }

    MEMZERO(servaddr);
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    //servaddr.sin_addr.s_addr = htonl(addr); //INADDR_LOOPBACK for 127.0.0.1
    servaddr.sin_addr.s_addr = inp.s_addr;
    servaddr.sin_port = htons(ip_port);

    // Binding newly created socket to given IP and verification
    if ((bind(srv_fd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        log_fatal("Socket bind failed... [%m]");
        return  -1;;
    }

    // Now server is ready to listen and verification
    if( listen(srv_fd, 1) != 0 ) {
        log_fatal("Server listen failed... [%m]");
        return -1;
    }

    return srv_fd;
}


/**
 * @param [in] srv_fd  Server's file descriptor
 * @param [in] addr string
 * @param [in] port string
 * @param [out] client_fd New Client's file descriptor
 * @return 0 if success
 * @return -1 if error
 */
int srv_accept_conn(int srv_fd, const char *addr_str, const char *port) {
    int client_fd;
    log_info("Server waiting for a client on %s:%s...", addr_str, port);

    // Accept the data packet from client and verification
    client_fd = accept(srv_fd, (struct sockaddr*)NULL, NULL);
    if( client_fd < 0 ) {
        log_fatal("Client acccept failed... [%m]");
        return -1;
    }

    log_info("Server acccept the client...");

    return client_fd;
}

/**********************
 * Unix socket Server *
 *********************/

/**
 * @param socket_path is string argument like "/tmp/srv.sock"
 * @return fd >0 if success
 * @return -1 if error
 */
int srv_unix_sock_init (char *socket_path)
{
    int server_sock, ret;
    int backlog = 10;
    struct sockaddr_un server_sockaddr;

    /**************************************/
    /* Create a UNIX domain stream socket */
    /**************************************/
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock == -1){
        log_fatal("socket(): [%m]");
        return -1;
    }

    /***************************************/
    /* Set up the UNIX sockaddr structure  */
    /* by using AF_UNIX for the family and */
    /* giving it a filepath to bind to.    */
    /*                                     */
    /* Unlink the file so the bind will    */
    /* succeed, then bind to that file.    */
    /***************************************/
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, socket_path);
    unsigned int len = sizeof(server_sockaddr);

    unlink(socket_path);
    ret = bind(server_sock, (struct sockaddr *) &server_sockaddr, len);
    if (ret == -1){
        log_fatal("bind(): [%m]");
        return -1;
    }

    /*********************************/
    /* Listen for any client sockets */
    /*********************************/
    ret = listen(server_sock, backlog);
    if (ret == -1){
        log_fatal("bind(): [%m]");
        close(server_sock);
        return -1;
    }
    log_info("Server waiting for a client...");

    return server_sock;
}




/******************* Send/Receive functions *******************/

int send_payload(int fd, uint8_t *buff, size_t buff_sz)
{
    size_t n_bytes = send(fd, buff, buff_sz, MSG_NOSIGNAL);
    if( n_bytes != buff_sz ) {
        log_fatal("TCP: Not able to sent %d bytes", buff_sz);
        return -1;
    }

    return 0;
}

/*
int srv_get_data(struct Srv_inst* i) {

    int n_bytes = recv(i->peer_fd, i->read_buff, sizeof(i->read_buff), 0);
    if( n_bytes == -1 ) {
        err("Some error with client [%m]");
        return -1;
    } else if( n_bytes == 0 ) {
        info("Client closed connection");
        return 0;
    }

    return n_bytes;
}
*/

/**
 * @param [in] fd  File descriptor to read from
 * @param [in] buff  Buffer to read into
 * @param [in] total_read  Read bytes counts
 * @return 0 if success
 * @return -1 if error
 */
ssize_t recv_payload(int fd, uint8_t *buff, size_t buff_sz, ssize_t total_read)
{
    struct pollfd pfds;
    int ret;
    ssize_t n_bytes;
    size_t offset = 0;
    size_t left_to_read;
    int iter = 0;

    if( total_read > 0 && total_read <= buff_sz ) {
        left_to_read = total_read;
    } else {
        left_to_read = buff_sz;
    }

    pfds.fd = fd;
    pfds.events = POLLIN;

    //double time_start = stopwatch(NULL, 0);

    while (1) {
        if (left_to_read == 0)
          break;

        ret = poll(&pfds, 1, POLL_TIMEOUT);
        if( ret == -1 ) {
            log_fatal("poll: [%m]");
            return -1;

        } else if( ret == 0 ) {
            log_fatal("poll: Time out");
            return -1;
        }
/*
        printf("  fd=%d; events: %s%s%s%s\n", pfds.fd,
               (pfds.revents & POLLIN)  ? "POLLIN "  : "",
               (pfds.revents & POLLHUP) ? "POLLHUP " : "",
               (pfds.revents & POLLHUP) ? "POLLRDHUP " : "",
               (pfds.revents & POLLERR) ? "POLLERR " : "");
*/
        if (pfds.revents & POLLIN) {
            n_bytes = recv(fd, buff + offset, left_to_read, 0);
            if( n_bytes == -1 ) {
                log_fatal("recv: [%m]");
                return -1;
            }
            if( n_bytes == 0 ) {
                log_warn("peer closed connection");
                return -1;
            }
        } else {  // POLLERR | POLLHUP
            log_warn("peer closed connection");
            return -1;
        }


        if( total_read == -1 )
            return n_bytes;

        offset += n_bytes;
        left_to_read -= n_bytes;
        log_trace("Iter = %d", ++iter);
    }

    //log_info("total_to_read = %d", total_to_read);
    return total_read;
}


/******************* Client's functions *******************/

/**
 * @param [in] addr as string
 * @param [in] port as string
 * @param [out] File descriptor for communicate with server
 * @return 0 if success
 * @return -1 if error
 */
int conn_to_srv(const char *addr_str, const char *port_str) {
    int client_fd;
    struct sockaddr_in servaddr;
    MEMZERO(servaddr);

    int port = (int)strtol(port_str, NULL, 10);

    struct in_addr inp;
    inet_aton(addr_str, &inp);

    // socket create and verification
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if( client_fd == -1 ) {
        log_fatal("Socket creation failed...");
        return -1;
    }
    log_trace("Socket successfully created..");

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inp.s_addr;
    servaddr.sin_port = htons(port);

    // connect the client socket to server socket
    int ret = connect(client_fd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret) {
        log_fatal("Connection with the server failed...");
        return -1;
    }
    log_info("Connected to the server..");

    return client_fd;
}



int conn_to_unix_socket(const char *socket_path) {
    int client_fd;
    struct sockaddr_un addr;
    MEMZERO(addr);

    // socket create and verification
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if( client_fd == -1 ) {
        log_fatal("Socket creation failed...");
        return -1;
    }
    log_info("Socket successfully created..");

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);
    //printf("addr.sun_path: %s [%lu] \n", addr.sun_path, strlen(addr.sun_path));

    // connect the client socket to server socket
    int ret = connect(client_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret) {
        log_fatal("Connection with the server failed...");
        return -1;
    }
    log_info("Connected to the server..");

    return client_fd;
}


/********************** Misc Functions **********************/

void close_fd(int fd, const char *str) {
    if( close(fd) == -1 )
        log_fatal(str);

    log_info(str);
}


/**
 * @param ipAddress is string argument like "129.168.1.1"
 * @return 0 if valid IP address; -1 in other cases
 */
static int isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int ret = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    if( ret != 1 )
        return -1;

    return 0;
}


/**
 * @param ipPort is string argument like "7777"
 * @return 0 if success; -1 if error
 */
static int isValidIpPort(char *ipPort) {
    int ip_port = (int)strtol(ipPort, NULL, 10);

    if (ip_port < 1 || ip_port > 65535) {
        return -1;
    }

    return 0;
}

/**
 * @param [in] argument string like "192.168.1.1:7777"
 * @param [out] ipaddr IP address string
 * @param [out] ipport IP port string
 * @return 0 if success
 * @return -1 if error
 */
int strToAddrPort(const char *argument, char *ipaddr, char *ipport) {
    int retcode = -1;

    char *str = strdup(argument);

    log_trace("%s(%s)", __FUNCTION__, str);
    char *colon_ptr = strchr(str, ':');
    if (!colon_ptr) {
        log_warn("There is no parameter IP:port");
        goto out;
    }
    *colon_ptr = '\0';

    if( isValidIpAddress(str) != 0 ) {
        log_warn("Malformed parameter 'IP'");
        goto out;
    }

    if( isValidIpPort(colon_ptr + 1) != 0 ) {
        log_warn("Malformed parameter 'Port'");
        goto out;
    }

    strcpy(ipaddr, str);
    strcpy(ipport, colon_ptr + 1);

    log_trace("ipaddr = '%s',  ipport = '%s'", ipaddr, ipport);
    retcode = 0;

out:
    free(str);

    return retcode;
}


