#ifndef TCP_SERVER_H
#define TCP_SERVER_H

int setup_tcp_connection(int port_nr, int *socket, int *fd);
void close_tcp_connection(int connfd);

#endif
