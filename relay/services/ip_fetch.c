
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#include <protocol/relay_data_structs.h>

static bool get_interface_sockstorage(struct sockaddr_storage* storage)
{
    struct ifaddrs *ifaddr, *ifa;
    socklen_t socklen;
    
    if (getifaddrs(&ifaddr) == -1) 
    {
        fprintf(stderr,"getifaddrs");
        return false;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
    {
        // Skip loopback interfaces
        if (ifa->ifa_addr == NULL || strcmp(ifa->ifa_name, "lo") == 0)
        {
            continue;
        }

        int family = ifa->ifa_addr->sa_family;

        if (family == AF_INET || family == AF_INET6) 
        {
            socklen = family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
            memcpy(storage, ifa->ifa_addr, socklen);
            
            freeifaddrs(ifaddr);
            return true;
        }
    }
    
    freeifaddrs(ifaddr);
    return false;
}


static void convert_sock_addr(sock_addr_t* result, const struct sockaddr_storage* storage)
{
    if (storage->ss_family == AF_INET) 
    {
        const struct sockaddr_in* addr4 = (const struct sockaddr_in*)storage;
        result->family = AF_INET;
        result->protocol = IPPROTO_TCP;
        memcpy(result->addr, &addr4->sin_addr, sizeof(struct in_addr));
        result->port = addr4->sin_port;
    }
    else if (storage->ss_family == AF_INET6) 
    {
        const struct sockaddr_in6* addr6 = (const struct sockaddr_in6*)storage;
        result->family = AF_INET6;
        result->protocol = IPPROTO_TCP;
        memcpy(result->addr, &addr6->sin6_addr, sizeof(struct in6_addr));
        result->port = addr6->sin6_port;
    }
    else 
    {
        memset(result, 0, sizeof(sock_addr_t));
    }
}

bool fetch_ip_sockaddr(sock_addr_t* result)
{
    struct sockaddr_storage addr;
    
    if (get_interface_sockstorage(&addr) == false)
    {
        return false;
    }

    convert_sock_addr(result, &addr);

    return true;
}