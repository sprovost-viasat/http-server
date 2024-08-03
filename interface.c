#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>

#include "interface.h"

void print_available_interfaces(unsigned int port)
{
    printf("Server is up and available through:\n");

    struct ifaddrs *ifap, *ifa;
    int family;

    getifaddrs(&ifap);

    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;
        if (family == AF_INET)
        {
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
            printf("%s - %s:%u\n",
                ifa->ifa_name,
                inet_ntoa(sa->sin_addr),
                port);
        }
    }

    freeifaddrs(ifap);

}