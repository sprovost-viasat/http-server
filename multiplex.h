#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>

typedef struct SocketAddress {
    int fd;
    struct sockaddr_in client;
} SocketAddress;

void init_monitor_fd_set(SocketAddress **monitored_fd_set, const int MAX_SUPPORTED_CLIENTS);
void add_to_monitored_fd_set(SocketAddress **monitored_fd_set, const int MAX_SUPPORTED_CLIENTS, int fd, struct sockaddr_in client);
void remove_from_monitored_fd_set(SocketAddress **monitored_fd_set, const int MAX_SUPPORTED_CLIENTS, int fd);
void re_init_fds(SocketAddress **monitored_fd_set, const int MAX_SUPPORTED_CLIENTS, fd_set *fdset);
int get_max_fd(SocketAddress **monitored_fd_set, const int MAX_SUPPORTED_CLIENTS);
