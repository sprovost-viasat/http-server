#include "multiplex.h"

void init_monitor_fd_set(SocketAddress **monitored_fd_set, const int MAX_SUPPORTED_CLIENTS)
{
    for (int i = 0; i < MAX_SUPPORTED_CLIENTS; i++)
    {
        monitored_fd_set[i]->fd = -1;
    }
}

void add_to_monitored_fd_set(SocketAddress **monitored_fd_set, const int MAX_SUPPORTED_CLIENTS, int fd, struct sockaddr_in client)
{
    for (int i = 0; i < MAX_SUPPORTED_CLIENTS; i++)
    {
        if (monitored_fd_set[i]->fd != -1)
        {
            continue;
        }
        monitored_fd_set[i]->fd = fd;
        monitored_fd_set[i]->client = client;
        break;
    }
}

void remove_from_monitored_fd_set(SocketAddress **monitored_fd_set, const int MAX_SUPPORTED_CLIENTS, int fd)
{
    for (int i = 0; i < MAX_SUPPORTED_CLIENTS; i++)
    {
        if (monitored_fd_set[i]->fd != fd)
        {
            continue;
        }
        monitored_fd_set[i]->fd = -1;
        break;
    }
}

void re_init_fds(SocketAddress **monitored_fd_set, const int MAX_SUPPORTED_CLIENTS, fd_set *fdset)
{
    FD_ZERO(fdset);
    for (int i = 0; i < MAX_SUPPORTED_CLIENTS; i++)
    {
        if (monitored_fd_set[i]->fd != -1)
        {
            FD_SET(monitored_fd_set[i]->fd, fdset);
        }
    }
}

int get_max_fd(SocketAddress **monitored_fd_set, const int MAX_SUPPORTED_CLIENTS)
{
    int max = -1;
    for (int i = 0; i < MAX_SUPPORTED_CLIENTS; i++)
    {
        if (monitored_fd_set[i]->fd != -1)
        {
            max = (monitored_fd_set[i]->fd > max) ? monitored_fd_set[i]->fd : max;
        }
    }
    return max;
}
